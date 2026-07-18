//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"

#include "RecluseFramework_exports.hpp"

namespace Recluse {

typedef void* FunctionHandle;

// Dynamic link library loader. Allows to proc addresses and handles clean up.
class R_OS_SPECIFIC DllLoader 
{
public:
    RecluseFramework_PUBLIC_API                 DllLoader(const std::string& dllName = "");
    RecluseFramework_PUBLIC_API                 ~DllLoader();

    RecluseFramework_PUBLIC_API Bool            isLoaded();

    RecluseFramework_PUBLIC_API Bool            load(const std::string& dllName);
    RecluseFramework_PUBLIC_API Bool            unload();

    RecluseFramework_PUBLIC_API FunctionHandle  procAddress(const std::string& name);

private:
    std::string     name;
    void*           library;
};
} // Recluse