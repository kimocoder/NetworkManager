/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* NetworkManager -- Network link manager
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright (C) 2014 Red Hat, Inc.
 */

#ifndef __NETWORKMANAGER_DEFAULT_ROUTE_MANAGER_H__
#define __NETWORKMANAGER_DEFAULT_ROUTE_MANAGER_H__

#include "nm-connection.h"

#define NM_TYPE_DEFAULT_ROUTE_MANAGER            (nm_default_route_manager_get_type ())
#define NM_DEFAULT_ROUTE_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NM_TYPE_DEFAULT_ROUTE_MANAGER, NMDefaultRouteManager))
#define NM_DEFAULT_ROUTE_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NM_TYPE_DEFAULT_ROUTE_MANAGER, NMDefaultRouteManagerClass))
#define NM_IS_DEFAULT_ROUTE_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NM_TYPE_DEFAULT_ROUTE_MANAGER))
#define NM_IS_DEFAULT_ROUTE_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NM_TYPE_DEFAULT_ROUTE_MANAGER))
#define NM_DEFAULT_ROUTE_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NM_TYPE_DEFAULT_ROUTE_MANAGER, NMDefaultRouteManagerClass))

#define NM_DEFAULT_ROUTE_MANAGER_LOG_WITH_PTR "log-with-ptr"
#define NM_DEFAULT_ROUTE_MANAGER_PLATFORM     "platform"

typedef struct _NMDefaultRouteManagerClass NMDefaultRouteManagerClass;

GType nm_default_route_manager_get_type (void);

NMDefaultRouteManager *nm_default_route_manager_new (gboolean log_with_ptr, NMPlatform *platform);

gboolean nm_default_route_manager_ip4_update_default_route (NMDefaultRouteManager *manager, gpointer source);
gboolean nm_default_route_manager_ip6_update_default_route (NMDefaultRouteManager *manager, gpointer source);

NMDevice *nm_default_route_manager_ip4_get_best_device (NMDefaultRouteManager *manager, const GSList *devices, gboolean fully_activated, NMDevice *preferred_device);
NMDevice *nm_default_route_manager_ip6_get_best_device (NMDefaultRouteManager *manager, const GSList *devices, gboolean fully_activated, NMDevice *preferred_device);

NMIP4Config *nm_default_route_manager_ip4_get_best_config (NMDefaultRouteManager *manager,
                                                           gboolean ignore_never_default,
                                                           const char **out_ip_iface,
                                                           NMActiveConnection **out_ac,
                                                           NMDevice **out_device,
                                                           NMVpnConnection **out_vpn);
NMIP6Config *nm_default_route_manager_ip6_get_best_config (NMDefaultRouteManager *manager,
                                                           gboolean ignore_never_default,
                                                           const char **out_ip_iface,
                                                           NMActiveConnection **out_ac,
                                                           NMDevice **out_device,
                                                           NMVpnConnection **out_vpn);

gboolean nm_default_route_manager_resync (NMDefaultRouteManager *self,
                                          int af_family);

#endif  /* NM_DEFAULT_ROUTE_MANAGER_H */
