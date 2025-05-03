#pragma once

//-----------------------------------------------------------------------------
// includes <...>
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// includes "..."
//-----------------------------------------------------------------------------
#include <memory>

namespace tev {

// Abstract Controller Class
class IController : public std::enable_shared_from_this<IController> {
 public:
  virtual ~IController() = default;
};

}  // end of namespace tev
