#include "BpeToDot.hpp"

#include <cstring>
#include <sstream>

namespace Garnish
{
    namespace BpeToDot
    {
        void renderDot(const Pairs &pairs, std::string &out)
        {
            std::ostringstream buf;

            buf << "digraph Pairs \n{\n";

            for(size_t token = 0; token < pairs.size(); ++token)
            {
                if(token != pairs[token].left)
                {
                    buf << "    " << token << " -> " << pairs[token].left << '\n';
                    buf << "    " << token << " -> " << pairs[token].right << '\n';
                }
            }

            buf << "}\n";

            out += buf.str();
        }
    }
}