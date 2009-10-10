/*
 * evd-socket.h
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

#ifndef __EVD_SOCKET_H__
#define __EVD_SOCKET_H__

#include <gio/gio.h>

#include "evd-stream.h"

G_BEGIN_DECLS

typedef struct _EvdSocket EvdSocket;
typedef struct _EvdSocketClass EvdSocketClass;
typedef struct _EvdSocketPrivate EvdSocketPrivate;
typedef struct _EvdSocketEvent EvdSocketEvent;

/**
 * EvdSocketReadHandler:
 * @socket: (in): The #EvdSocket
 * @user_data: (in) (allow-null): A #gpointer to user defined data to pass in callback.
 *
 * Prototype for callback to be executed when 'read' event is received on the socket.
 */
typedef void (* EvdSocketReadHandler) (EvdSocket *socket,
				       gpointer   user_data);

struct _EvdSocket
{
  EvdStream parent;

  /* private structure */
  EvdSocketPrivate *priv;
};

struct _EvdSocketClass
{
  EvdStreamClass parent_class;

  /* virtual methods */
  gboolean (* event_handler) (EvdSocket *self, GIOCondition condition);

  /* signal prototypes */
  void (* error)           (EvdSocket *self,
			    GError    *error);
  void (* close)           (EvdSocket *self);
  void (* connect)         (EvdSocket *self);
  void (* bind)            (EvdSocket      *self,
			    GSocketAddress *address);
  void (* listen)          (EvdSocket *self);
  void (* new_connection)  (EvdSocket *self,
			    EvdSocket *socket);
  void (* connect_timeout) (EvdSocket *self);
};

/* event message to pass to sockets objects*/
struct _EvdSocketEvent
{
  GIOCondition  condition;
  EvdSocket    *socket;
};

/* socket states */
typedef enum
{
  EVD_SOCKET_CLOSED,
  EVD_SOCKET_CONNECTING,
  EVD_SOCKET_CONNECTED,
  EVD_SOCKET_BINDING,
  EVD_SOCKET_BOUND,
  EVD_SOCKET_LISTENING
} EvdSocketState;

/* error codes */
typedef enum
{
  EVD_SOCKET_ERROR_UNKNOWN,
  EVD_SOCKET_ERROR_NOT_CONNECTING,
  EVD_SOCKET_ERROR_NOT_CONNECTED,
  EVD_SOCKET_ERROR_NOT_BOUND,
  EVD_SOCKET_ERROR_CLOSE,
  EVD_SOCKET_ERROR_ACCEPT,
  EVD_SOCKET_ERROR_LAST
} EvdSocketError;

#define EVD_TYPE_SOCKET           (evd_socket_get_type ())
#define EVD_SOCKET(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), EVD_TYPE_SOCKET, EvdSocket))
#define EVD_SOCKET_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST ((obj), EVD_TYPE_SOCKET, EvdSocketClass))
#define EVD_IS_SOCKET(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EVD_TYPE_SOCKET))
#define EVD_IS_SOCKET_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE ((obj), EVD_TYPE_SOCKET))
#define EVD_SOCKET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EVD_TYPE_SOCKET, EvdSocketClass))


GType evd_socket_get_type (void) G_GNUC_CONST;

EvdSocket    *evd_socket_new              (void);
EvdSocket    *evd_socket_new_from_fd      (gint     fd,
					   GError **error);

GSocket      *evd_socket_get_socket       (EvdSocket *self);
GMainContext *evd_socket_get_context      (EvdSocket *self);
GSocketFamily evd_socket_get_family       (EvdSocket *self);
EvdSocketState evd_socket_get_status      (EvdSocket *self);


gboolean      evd_socket_close            (EvdSocket *self, GError **error);

gboolean      evd_socket_bind             (EvdSocket       *self,
					   GSocketAddress  *address,
					   gboolean         allow_reuse,
					   GError         **error);
gboolean      evd_socket_listen           (EvdSocket *self, GError **error);
EvdSocket    *evd_socket_accept           (EvdSocket *socket, GError **error);
gboolean      evd_socket_connect_to       (EvdSocket       *self,
					   GSocketAddress  *address,
					   GError         **error);
gboolean      evd_socket_cancel_connect   (EvdSocket *self, GError **error);

void          evd_socket_set_read_handler (EvdSocket            *self,
					   EvdSocketReadHandler  handler,
					   gpointer              user_data);

/**
 * evd_socket_read_to_buffer:
 * @self: The #EvdSocket
 * @buffer: (utf8) (out) (transfer full): The buffer to store the data.
 * @size: Maximum number of bytes to read.
 * @error: (out): The error to catch.
 *
 * Return value: The actual size of bytes read.
 */
gssize        evd_socket_read_to_buffer   (EvdSocket *self,
					   gchar     *buffer,
					   gsize      size,
					   GError   **error);

/**
 * evd_socket_read:
 * @self: The #EvdSocket
 * @size: (inout): Maximum number of bytes to read.
 * @error: (out) (transfer full): The error to catch.
 *
 * Return value: (transfer full): The #GString representing the read package.
 */
gchar         *evd_socket_read            (EvdSocket *self,
					   gsize     *size,
					   GError   **error);

gssize        evd_socket_send             (EvdSocket    *socket,
					   const gchar  *buf,
					   gsize         size,
					   GError      **error);

G_END_DECLS

#endif /* __EVD_SOCKET_H__ */
