#include <ostream>

#include "address.h"

namespace agent{

    std::ostream& operator<<(std::ostream& os, const Socket& addr);
}