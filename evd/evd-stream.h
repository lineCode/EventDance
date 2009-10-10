/*
 * evd-stream.h
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

#ifndef __EVD_STREAM_H__
#define __EVD_STREAM_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _EvdStream EvdStream;
typedef struct _EvdStreamClass EvdStreamClass;
typedef struct _EvdStreamPrivate EvdStreamPrivate;

struct _EvdStream
{
  GObject parent;

  /* private structure */
  EvdStreamPrivate *priv;
};

struct _EvdStreamClass
{
  GObjectClass parent_class;

  /* virtual methods */

  /* signal prototypes */
};

#define EVD_TYPE_STREAM           (evd_stream_get_type ())
#define EVD_STREAM(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), EVD_TYPE_STREAM, EvdStream))
#define EVD_STREAM_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST ((obj), EVD_TYPE_STREAM, EvdStreamClass))
#define EVD_IS_STREAM(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EVD_TYPE_STREAM))
#define EVD_IS_STREAM_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE ((obj), EVD_TYPE_STREAM))
#define EVD_STREAM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EVD_TYPE_STREAM, EvdStreamClass))


GType evd_stream_get_type (void) G_GNUC_CONST;

EvdStream    *evd_stream_new            (void);

void          evd_stream_set_on_receive (EvdStream *self,
					 GClosure  *closure);
GClosure     *evd_stream_get_on_receive (EvdStream *self);

G_END_DECLS

#endif /* __EVD_STREAM_H__ */