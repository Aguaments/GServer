#include "socket.h"

namespace agent{

    std::ostream& operator<<(std::ostream& os, const Socket& addr){
        return addr -> dump(os);
    }
}