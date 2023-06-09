//* Copyright 2019 The Dawn Authors
//*
//* Licensed under the Apache License, Version 2.0 (the "License");
//* you may not use this file except in compliance with the License.
//* You may obtain a copy of the License at
//*
//*     http://www.apache.org/licenses/LICENSE-2.0
//*
//* Unless required by applicable law or agreed to in writing, software
//* distributed under the License is distributed on an "AS IS" BASIS,
//* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//* See the License for the specific language governing permissions and
//* limitations under the License.

#ifndef DAWNNATIVE_OPENGL_OPENGLFUNCTIONSBASE_H_
#define DAWNNATIVE_OPENGL_OPENGLFUNCTIONSBASE_H_

#include <unordered_set>

#include "dawn/native/Error.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {
    using GetProcAddress = void* (*) (const char*);

    struct OpenGLFunctionsBase {
      public:
        {% for block in header_blocks %}
            // {{block.description}}
            {% for proc in block.procs %}
                {{proc.PFNGLPROCNAME()}} {{proc.ProcName()}} = nullptr;
            {% endfor %}

        {% endfor%}

        // GL_ANGLE_base_vertex_base_instance
        // See crbug.com/dawn/1715 for why this is embedded
        PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEANGLEPROC DrawArraysInstancedBaseInstanceANGLE = nullptr;
        PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEANGLEPROC DrawElementsInstancedBaseVertexBaseInstanceANGLE = nullptr;
        PFNGLMULTIDRAWARRAYSINSTANCEDBASEINSTANCEANGLEPROC MultiDrawArraysInstancedBaseInstanceANGLE = nullptr;
        PFNGLMULTIDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEANGLEPROC MultiDrawElementsInstancedBaseVertexBaseInstanceANGLE = nullptr;

        bool IsGLExtensionSupported(const char* extension) const;

      protected:
        MaybeError LoadDesktopGLProcs(GetProcAddress getProc, int majorVersion, int minorVersion);
        MaybeError LoadOpenGLESProcs(GetProcAddress getProc, int majorVersion, int minorVersion);

      private:
        template<typename T>
        MaybeError LoadProc(GetProcAddress getProc, T* memberProc, const char* name);
        void InitializeSupportedGLExtensions();

        std::unordered_set<std::string> mSupportedGLExtensionsSet;
    };

}  // namespace dawn::native::opengl

#endif // DAWNNATIVE_OPENGL_OPENGLFUNCTIONSBASE_H_
