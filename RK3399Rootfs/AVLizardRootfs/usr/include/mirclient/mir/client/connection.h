/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2 or 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef MIR_CLIENT_CONNECTION_H
#define MIR_CLIENT_CONNECTION_H

#include <mir_toolkit/mir_connection.h>

#include <memory>

namespace mir
{
/// Convenient C++ wrappers around the Mir toolkit API.
///
/// These wrappers are intentionally inline adapters: the compiled code depend directly on the Mir toolkit API.
namespace client
{
/// Handle class for MirConnection - provides automatic reference counting.
class Connection
{
public:
    Connection() = default;
    explicit Connection(MirConnection* connection) : self{connection, deleter} {}

    operator MirConnection*() const { return self.get(); }

    void reset() { self.reset(); }

private:
    static void deleter(MirConnection* connection) { mir_connection_release(connection); }
    std::shared_ptr<MirConnection> self;
};

// Provide a deleted overload to avoid double release "accidents".
void mir_connection_release(Connection const& connection) = delete;
}
}

#endif //MIR_CLIENT_CONNECTION_H
