#pragma once

//-----------------------------------------------------------------------------
// includes <...>
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// includes "..."
//-----------------------------------------------------------------------------

#include <stdint.h>

#include <any>
#include <memory>

namespace tev {

/// Abstract interface for user-defined event data
class IUserData : public std::enable_shared_from_this<IUserData> {
 public:
  virtual ~IUserData() = default;
};

}  // end of namespace tev
