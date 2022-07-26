// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// gen parses the <tint>/src/tint/intrinsics.def file, then scans the
// project directory for '<file>.tmpl' files, to produce source code files.
package main

import (
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"reflect"
	"strings"
	"text/template"
	"unicode"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/gen"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/parser"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/resolver"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/sem"
)

const defProjectRelPath = "src/tint/intrinsics.def"

func main() {
	if err := run(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println(`
gen generates the templated code for the Tint compiler

gen scans the project directory for '<file>.tmpl' files, to produce source code
files.

If the templates use the 'IntrinsicTable' function then gen will parse and
resolve the <tint>/src/tint/intrinsics.def file.

usage:
  gen

optional flags:`)
	flag.PrintDefaults()
	fmt.Println(``)
	os.Exit(1)
}

func run() error {
	projectRoot := fileutils.ProjectRoot()

	// Recursively find all the template files in the <tint>/src directory
	files, err := glob.Scan(projectRoot, glob.MustParseConfig(`{
		"paths": [{"include": [
			"src/tint/**.tmpl",
			"test/tint/**.tmpl"
		]}]
	}`))
	if err != nil {
		return err
	}

	// For each template file...
	for _, relTmplPath := range files {
		// Make tmplPath absolute
		tmplPath := filepath.Join(projectRoot, relTmplPath)

		// Read the template file
		tmpl, err := ioutil.ReadFile(tmplPath)
		if err != nil {
			return fmt.Errorf("failed to open '%v': %w", tmplPath, err)
		}

		// Create or update the file at relpath if the file content has changed
		// relpath is a path relative to the template
		writeFile := func(relpath, body string) error {
			// Write the common file header
			sb := strings.Builder{}
			sb.WriteString(fmt.Sprintf(header, filepath.ToSlash(relTmplPath)))
			sb.WriteString(body)
			content := sb.String()
			abspath := filepath.Join(filepath.Dir(tmplPath), relpath)
			return writeFileIfChanged(abspath, content)
		}

		// Write the content generated using the template and semantic info
		sb := strings.Builder{}
		if err := generate(string(tmpl), &sb, writeFile); err != nil {
			return fmt.Errorf("while processing '%v': %w", tmplPath, err)
		}

		if body := sb.String(); body != "" {
			_, tmplFileName := filepath.Split(tmplPath)
			outFileName := strings.TrimSuffix(tmplFileName, ".tmpl")
			if err := writeFile(outFileName, body); err != nil {
				return err
			}
		}
	}

	return nil
}

// writes content to path if the file has changed
func writeFileIfChanged(path, content string) error {
	existing, err := ioutil.ReadFile(path)
	if err == nil && string(existing) == content {
		return nil // Not changed
	}
	if err := os.MkdirAll(filepath.Dir(path), 0777); err != nil {
		return fmt.Errorf("failed to create directory for '%v': %w", path, err)
	}
	if err := ioutil.WriteFile(path, []byte(content), 0666); err != nil {
		return fmt.Errorf("failed to write file '%v': %w", path, err)
	}
	return nil
}

const header = `// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

////////////////////////////////////////////////////////////////////////////////
// File generated by tools/src/cmd/gen
// using the template:
//   %v
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

`

type generator struct {
	t      *template.Template
	cached struct {
		sem            *sem.Sem            // lazily built by sem()
		intrinsicTable *gen.IntrinsicTable // lazily built by intrinsicTable()
		permuter       *gen.Permuter       // lazily built by permute()
	}
}

// WriteFile is a function that Generate() may call to emit a new file from a
// template.
// relpath is the relative path from the currently executing template.
// content is the file content to write.
type WriteFile func(relpath, content string) error

// generate executes the template tmpl, writing the output to w.
// See https://golang.org/pkg/text/template/ for documentation on the template
// syntax.
func generate(tmpl string, w io.Writer, writeFile WriteFile) error {
	g := generator{}
	return g.generate(tmpl, w, writeFile)
}

func (g *generator) generate(tmpl string, w io.Writer, writeFile WriteFile) error {
	t, err := template.New("<template>").Funcs(map[string]interface{}{
		"Map":                   newMap,
		"Iterate":               iterate,
		"Title":                 strings.Title,
		"PascalCase":            pascalCase,
		"SplitDisplayName":      gen.SplitDisplayName,
		"HasPrefix":             strings.HasPrefix,
		"HasSuffix":             strings.HasSuffix,
		"TrimPrefix":            strings.TrimPrefix,
		"TrimSuffix":            strings.TrimSuffix,
		"TrimLeft":              strings.TrimLeft,
		"TrimRight":             strings.TrimRight,
		"IsEnumEntry":           is(sem.EnumEntry{}),
		"IsEnumMatcher":         is(sem.EnumMatcher{}),
		"IsFQN":                 is(sem.FullyQualifiedName{}),
		"IsInt":                 is(1),
		"IsTemplateEnumParam":   is(sem.TemplateEnumParam{}),
		"IsTemplateNumberParam": is(sem.TemplateNumberParam{}),
		"IsTemplateTypeParam":   is(sem.TemplateTypeParam{}),
		"IsType":                is(sem.Type{}),
		"IsAbstract":            gen.IsAbstract,
		"IsDeclarable":          gen.IsDeclarable,
		"IsFirstIn":             isFirstIn,
		"IsLastIn":              isLastIn,
		"Sem":                   g.sem,
		"IntrinsicTable":        g.intrinsicTable,
		"Permute":               g.permute,
		"Eval":                  g.eval,
		"WriteFile":             func(relpath, content string) (string, error) { return "", writeFile(relpath, content) },
	}).Option("missingkey=error").
		Parse(tmpl)
	if err != nil {
		return err
	}
	g.t = t
	return t.Execute(w, map[string]interface{}{})
}

// eval executes the sub-template with the given name and argument, returning
// the generated output
func (g *generator) eval(template string, args ...interface{}) (string, error) {
	target := g.t.Lookup(template)
	if target == nil {
		return "", fmt.Errorf("template '%v' not found", template)
	}
	sb := strings.Builder{}

	var err error
	if len(args) == 1 {
		err = target.Execute(&sb, args[0])
	} else {
		m := newMap()
		if len(args)%2 != 0 {
			return "", fmt.Errorf("Eval expects a single argument or list name-value pairs")
		}
		for i := 0; i < len(args); i += 2 {
			name, ok := args[i].(string)
			if !ok {
				return "", fmt.Errorf("Eval argument %v is not a string", i)
			}
			m.Put(name, args[i+1])
		}
		err = target.Execute(&sb, m)
	}

	if err != nil {
		return "", fmt.Errorf("while evaluating '%v': %v", template, err)
	}
	return sb.String(), nil
}

// sem lazily parses and resolves the intrinsic.def file, returning the semantic info.
func (g *generator) sem() (*sem.Sem, error) {
	if g.cached.sem == nil {
		// Load the builtins definition file
		defPath := filepath.Join(fileutils.ProjectRoot(), defProjectRelPath)

		defSource, err := ioutil.ReadFile(defPath)
		if err != nil {
			return nil, err
		}

		// Parse the definition file to produce an AST
		ast, err := parser.Parse(string(defSource), defProjectRelPath)
		if err != nil {
			return nil, err
		}

		// Resolve the AST to produce the semantic info
		sem, err := resolver.Resolve(ast)
		if err != nil {
			return nil, err
		}

		g.cached.sem = sem
	}
	return g.cached.sem, nil
}

// intrinsicTable lazily calls and returns the result of buildIntrinsicTable(),
// caching the result for repeated calls.
func (g *generator) intrinsicTable() (*gen.IntrinsicTable, error) {
	if g.cached.intrinsicTable == nil {
		sem, err := g.sem()
		if err != nil {
			return nil, err
		}
		g.cached.intrinsicTable, err = gen.BuildIntrinsicTable(sem)
		if err != nil {
			return nil, err
		}
	}
	return g.cached.intrinsicTable, nil
}

// permute lazily calls buildPermuter(), caching the result for repeated
// calls, then passes the argument to Permutator.Permute()
func (g *generator) permute(overload *sem.Overload) ([]gen.Permutation, error) {
	if g.cached.permuter == nil {
		sem, err := g.sem()
		if err != nil {
			return nil, err
		}
		g.cached.permuter, err = gen.NewPermuter(sem)
		if err != nil {
			return nil, err
		}
	}
	return g.cached.permuter.Permute(overload)
}

// Map is a simple generic key-value map, which can be used in the template
type Map map[interface{}]interface{}

func newMap() Map { return Map{} }

// Put adds the key-value pair into the map.
// Put always returns an empty string so nothing is printed in the template.
func (m Map) Put(key, value interface{}) string {
	m[key] = value
	return ""
}

// Get looks up and returns the value with the given key. If the map does not
// contain the given key, then nil is returned.
func (m Map) Get(key interface{}) interface{} {
	return m[key]
}

// is returns a function that returns true if the value passed to the function
// matches the type of 'ty'.
func is(ty interface{}) func(interface{}) bool {
	rty := reflect.TypeOf(ty)
	return func(v interface{}) bool {
		ty := reflect.TypeOf(v)
		return ty == rty || ty == reflect.PtrTo(rty)
	}
}

// isFirstIn returns true if v is the first element of the given slice.
func isFirstIn(v, slice interface{}) bool {
	s := reflect.ValueOf(slice)
	count := s.Len()
	if count == 0 {
		return false
	}
	return s.Index(0).Interface() == v
}

// isFirstIn returns true if v is the last element of the given slice.
func isLastIn(v, slice interface{}) bool {
	s := reflect.ValueOf(slice)
	count := s.Len()
	if count == 0 {
		return false
	}
	return s.Index(count-1).Interface() == v
}

// iterate returns a slice of length 'n', with each element equal to its index.
// Useful for: {{- range Iterate $n -}}<this will be looped $n times>{{end}}
func iterate(n int) []int {
	out := make([]int, n)
	for i := range out {
		out[i] = i
	}
	return out
}

// pascalCase returns the snake-case string s transformed into 'PascalCase',
// Rules:
// * The first letter of the string is capitalized
// * Characters following an underscore or number are capitalized
// * Underscores are removed from the returned string
// See: https://en.wikipedia.org/wiki/Camel_case
func pascalCase(s string) string {
	b := strings.Builder{}
	upper := true
	for _, r := range s {
		if r == '_' {
			upper = true
			continue
		}
		if upper {
			b.WriteRune(unicode.ToUpper(r))
			upper = false
		} else {
			b.WriteRune(r)
		}
		if unicode.IsNumber(r) {
			upper = true
		}
	}
	return b.String()
}