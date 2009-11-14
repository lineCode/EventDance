/*
 * evd-socket-group.c
 *
 * EventDance project - An event distribution framework (http://eventdance.org)
 *
 * Copyright (C) 2009, Igalia S.L.
 *
 * Authors:
 *   Eduardo Lima Mitev <elima@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include "evd-socket-protected.h"
#include "evd-socket-group.h"
#include "evd-socket-group-protected.h"

G_DEFINE_TYPE (EvdSocketGroup, evd_socket_group, EVD_TYPE_STREAM)

#define EVD_SOCKET_GROUP_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
	                                   EVD_TYPE_SOCKET_GROUP, \
                                           EvdSocketGroupPrivate))

/* private data */
struct _EvdSocketGroupPrivate
{
  GClosure *on_read_closure;
};

static void     evd_socket_group_class_init         (EvdSocketGroupClass *class);
static void     evd_socket_group_init               (EvdSocketGroup *self);

static void     evd_socket_group_finalize           (GObject *obj);
static void     evd_socket_group_dispose            (GObject *obj);

static void
evd_socket_group_class_init (EvdSocketGroupClass *class)
{
  GObjectClass *obj_class;

  obj_class = G_OBJECT_CLASS (class);

  obj_class->dispose = evd_socket_group_dispose;
  obj_class->finalize = evd_socket_group_finalize;

  class->add = evd_socket_group_add_internal;
  class->remove = evd_socket_group_remove_internal;

  /* add private structure */
  g_type_class_add_private (obj_class, sizeof (EvdSocketGroupPrivate));

}

static void
evd_socket_group_init (EvdSocketGroup *self)
{
  EvdSocketGroupPrivate *priv;

  priv = EVD_SOCKET_GROUP_GET_PRIVATE (self);
  self->priv = priv;

  /* initialize private members */
  priv->on_read_closure = NULL;
}

static void
evd_socket_group_dispose (GObject *obj)
{
  G_OBJECT_CLASS (evd_socket_group_parent_class)->dispose (obj);
}

static void
evd_socket_group_finalize (GObject *obj)
{
  G_OBJECT_CLASS (evd_socket_group_parent_class)->finalize (obj);
}

static void
evd_socket_group_socket_on_read (EvdSocket *socket, gpointer user_data)
{
  EvdSocketGroup *self = user_data;
  GClosure *closure = NULL;

  closure = evd_stream_get_on_read (EVD_STREAM (self));
  if (closure != NULL)
    {
      GValue params[2] = { {0, } };

      g_value_init (&params[0], EVD_TYPE_SOCKET_GROUP);
      g_value_set_object (&params[0], self);

      g_value_init (&params[1], EVD_TYPE_SOCKET);
      g_value_set_object (&params[1], socket);

      g_object_ref (self);
      g_closure_invoke (closure, NULL, 2, params, NULL);
      g_object_unref (self);

      g_value_unset (&params[0]);
      g_value_unset (&params[1]);
    }
}

/* protected methods */

void
evd_socket_group_add_internal (EvdSocketGroup *self,
			       EvdSocket      *socket)
{
  evd_socket_set_read_handler (socket,
			       evd_socket_group_socket_on_read,
			       self);
  evd_socket_set_group (socket, self);
}

gboolean
evd_socket_group_remove_internal (EvdSocketGroup *self,
				  EvdSocket      *socket)
{
  if (evd_socket_get_group (socket) == self)
    {
      evd_socket_set_read_handler (socket, NULL, NULL);
      evd_socket_set_group (socket, NULL);

      return TRUE;
    }
  else
    return FALSE;
}

/* public methods */

EvdSocketGroup *
evd_socket_group_new (void)
{
  EvdSocketGroup *self;

  self = g_object_new (EVD_TYPE_SOCKET_GROUP, NULL);

  return self;
}

void
evd_socket_group_add (EvdSocketGroup *self, EvdSocket *socket)
{
  EvdSocketGroupClass *class;

  g_return_if_fail (EVD_IS_SOCKET_GROUP (self));
  g_return_if_fail (EVD_IS_SOCKET (socket));

  class = EVD_SOCKET_GROUP_GET_CLASS (self);
  if (class->add != NULL)
    class->add (self, socket);
  else
    evd_socket_group_add_internal (self, socket);
}

gboolean
evd_socket_group_remove (EvdSocketGroup *self, EvdSocket *socket)
{
  EvdSocketGroupClass *class;

  g_return_val_if_fail (EVD_IS_SOCKET_GROUP (self), FALSE);
  g_return_val_if_fail (EVD_IS_SOCKET (socket), FALSE);

  class = EVD_SOCKET_GROUP_GET_CLASS (self);
  if (class->remove != NULL)
    return class->remove (self, socket);
  else
    return evd_socket_group_remove_internal (self, socket);
}

void
evd_socket_group_set_read_handler (EvdSocketGroup            *self,
				   EvdSocketGroupReadHandler  handler,
				   gpointer                   user_data)
{
  GClosure *closure;

  g_return_if_fail (EVD_IS_SOCKET_GROUP (self));

  closure = g_cclosure_new (G_CALLBACK (handler),
			    user_data,
			    NULL);

  if (G_CLOSURE_NEEDS_MARSHAL (closure))
    {
      GClosureMarshal marshal = g_cclosure_marshal_VOID__BOXED;

      g_closure_set_marshal (closure, marshal);
    }

  g_closure_ref (closure);
  g_closure_sink (closure);

  evd_stream_set_on_read (EVD_STREAM (self), closure);
}
