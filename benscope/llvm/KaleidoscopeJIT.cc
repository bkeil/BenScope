// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "benscope/llvm/KaleidoscopeJIT.h"

namespace llvm::orc {

KaleidoscopeJIT::KaleidoscopeJIT()
    : Resolver(createLegacyLookupResolver(
          ES,
          [this](StringRef Name) {
            return findMangledSymbol(std::string(Name));
          },
          [](Error Err) { cantFail(std::move(Err), "lookupFlags failed"); })),
      TM(EngineBuilder().selectTarget()), DL(TM->createDataLayout()),
      ObjectLayer(AcknowledgeORCv1Deprecation, ES,
                  [this](VModuleKey) {
                    return ObjLayerT::Resources{
                        std::make_shared<SectionMemoryManager>(), Resolver};
                  }),
      CompileLayer(AcknowledgeORCv1Deprecation, ObjectLayer,
                   SimpleCompiler(*TM)) {
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

} // namespace llvm::orc