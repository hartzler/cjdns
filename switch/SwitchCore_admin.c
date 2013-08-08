/* vim: set expandtab ts=4 sw=4: */
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "admin/Admin.h"
#include "benc/String.h"
#include "benc/Dict.h"
#include "benc/List.h"
#include "benc/Int.h"
#include "switch/SwitchCore.h"
#include "util/Endian.h"

struct Context
{
    struct Allocator* alloc;
    struct SwitchCore* core;
    struct Admin* admin;
};

static void trafficStats(Dict* args, void* vcontext, String* txid)
{
    struct Context* context = vcontext;
    uint64_t stats[256*2];

    uint32_t count = SwitchCore_trafficStats(context->core, stats);

    List* list = NULL;
    for (uint32_t i = 0; i < count; i++) {
      list = List_addInt(list, stats[i*2], context->alloc);
      list = List_addInt(list, stats[i*2+1], context->alloc);
    }

    Dict response = Dict_CONST(
        String_CONST("traffic"),
        List_OBJ(list), NULL
    );

    response = Dict_CONST(
        String_CONST("count"),
        Int_OBJ(count), response
    );

    Admin_sendMessage(&response, txid, context->admin);
}

void SwitchCore_admin_register(struct SwitchCore* core,
                               struct Admin* admin,
                               struct Allocator* alloc)
{
    struct Context* ctx = Allocator_clone(alloc, (&(struct Context) {
        .alloc = alloc,
        .core = core,
        .admin = admin
    }));

    Admin_registerFunction("SwitchCore_trafficStats", trafficStats, ctx, true,
        ((struct Admin_FunctionArg[]) {
            { .name = "index", .required = 0, .type = "Int" }
        }), admin);
}
