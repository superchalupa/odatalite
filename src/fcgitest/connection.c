/*
**==============================================================================
**
** ODatatLite ver. 0.0.3
**
** Copyright (c) Microsoft Corporation
**
** All rights reserved.
**
** MIT License
**
** Permission is hereby granted, free of charge, to any person obtaining a copy ** of this software and associated documentation files (the ""Software""), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions: The above copyright ** notice and this permission notice shall be included in all copies or
** substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
**
**==============================================================================
*/
#include "common.h"

#include "fcgitest/connection.h"
#include "phit.h"

#if 0

    /* Resolve the role for this user */
    {
        *role = RoleResolve(headers->username);

        if (*role == PHIT_ROLE_NONE)
        {
            LOGW(("Role authentication failed: %s", headers->username));
            goto AccessDenied;
        }

        LOGI(("authenticated %s as %s", headers->username, RoleName(*role)));
    }

#endif /* defined(ENABLE_PAM_AUTH) */

Connection* ConnectionNew()
{
    Connection* self;

    if (!(self = (Connection*)Calloc(1, sizeof(Connection))))
        return NULL;

    self->magic = CONNECTION_MAGIC;

    ContextInit(&self->context, self);

    return self;
}

void ConnectionDelete(
    Connection* self)
{
    LOGD(("ConnectionDelete(%p)", self));

    ContextDestroy(&self->context);

    self->magic = 0xDDDDDDDD;
    Free(self);
}
