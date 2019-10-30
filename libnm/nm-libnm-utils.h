// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright (C) 2017, 2018 Red Hat, Inc.
 */

#ifndef __NM_LIBNM_UTILS_H__
#define __NM_LIBNM_UTILS_H__

#include "c-list/src/c-list.h"
#include "nm-glib-aux/nm-ref-string.h"
#include "nm-types.h"
#include "nm-object.h"
#include "nm-client.h"

/*****************************************************************************/

/* Markers for deprecated sync code in internal API. */
#define _NM_DEPRECATED_SYNC_METHOD_INTERNAL            NM_DEPRECATED_IN_1_22
#define _NM_DEPRECATED_SYNC_WRITABLE_PROPERTY_INTERNAL NM_DEPRECATED_IN_1_22

/*****************************************************************************/

char *nm_utils_fixup_vendor_string (const char *desc);
char *nm_utils_fixup_product_string (const char *desc);

char *nm_utils_wincaps_to_dash (const char *caps);

gboolean nm_utils_g_param_spec_is_default (const GParamSpec *pspec);

/*****************************************************************************/

NMClientPermission nm_permission_to_client (const char *nm);

NMClientPermissionResult nm_permission_result_to_client (const char *nm);

/*****************************************************************************/

typedef enum {
	_NML_DBUS_LOG_LEVEL_INITIALIZED = 0x01,

	_NML_DBUS_LOG_LEVEL_TRACE       = 0x02,

	_NML_DBUS_LOG_LEVEL_DEBUG       = 0x04,

	/* the difference between a warning and a critical is that it results in
	 * g_warning() vs. g_critical() messages. Note that we want to use "warnings"
	 * for unknown D-Bus API that could just result because we run against a
	 * newer NetworkManager version (such warnings are more graceful, because
	 * we want that libnm can be forward compatible against newer servers).
	 * Critial warnings should be emitted when NetworkManager exposes something
	 * on D-Bus that breaks the current expectations. Usually NetworkManager
	 * should not break API, hence such issues are more severe. */
	_NML_DBUS_LOG_LEVEL_WARN        = 0x08,
	_NML_DBUS_LOG_LEVEL_ERROR       = 0x10,

	/* ANY is only relevant for nml_dbus_log_enabled() to check whether any of the
	 * options is on. */
	NML_DBUS_LOG_LEVEL_ANY          = _NML_DBUS_LOG_LEVEL_INITIALIZED,

	NML_DBUS_LOG_LEVEL_TRACE        = _NML_DBUS_LOG_LEVEL_TRACE,
	NML_DBUS_LOG_LEVEL_DEBUG        =   _NML_DBUS_LOG_LEVEL_DEBUG
	                                  | NML_DBUS_LOG_LEVEL_TRACE,
	NML_DBUS_LOG_LEVEL_WARN         =   _NML_DBUS_LOG_LEVEL_WARN
	                                  | NML_DBUS_LOG_LEVEL_DEBUG,
	NML_DBUS_LOG_LEVEL_ERROR        =   _NML_DBUS_LOG_LEVEL_ERROR
	                                  | NML_DBUS_LOG_LEVEL_WARN,
} NMLDBusLogLevel;

extern volatile int _nml_dbus_log_level;

int _nml_dbus_log_level_init (void);

static inline gboolean
nml_dbus_log_enabled (NMLDBusLogLevel level)
{
	int l;

	nm_assert (NM_IN_SET (level, NML_DBUS_LOG_LEVEL_ANY,
	                             NML_DBUS_LOG_LEVEL_TRACE,
	                             NML_DBUS_LOG_LEVEL_DEBUG,
	                             NML_DBUS_LOG_LEVEL_WARN,
	                             NML_DBUS_LOG_LEVEL_ERROR));

	l = g_atomic_int_get (&_nml_dbus_log_level);
	if (G_UNLIKELY (l == 0))
		l = _nml_dbus_log_level_init ();

	nm_assert (l & _NML_DBUS_LOG_LEVEL_INITIALIZED);
	if (level == NML_DBUS_LOG_LEVEL_ANY)
		return l != _NML_DBUS_LOG_LEVEL_INITIALIZED;
	return !!(((NMLDBusLogLevel) l) & level);
}

void _nml_dbus_log (NMLDBusLogLevel level,
                    const char *fmt,
                    ...) _nm_printf (2, 3);

#define NML_DBUS_LOG(level, ...) \
	G_STMT_START { \
		G_STATIC_ASSERT (   (level) == NML_DBUS_LOG_LEVEL_TRACE \
		                 || (level) == NML_DBUS_LOG_LEVEL_DEBUG \
		                 || (level) == NML_DBUS_LOG_LEVEL_WARN \
		                 || (level) == NML_DBUS_LOG_LEVEL_ERROR); \
		\
		if (nml_dbus_log_enabled (level)) { \
			_nml_dbus_log ((level), __VA_ARGS__); \
		} \
	} G_STMT_END

#define NML_DBUS_LOG_T(...) NML_DBUS_LOG (NML_DBUS_LOG_LEVEL_TRACE, __VA_ARGS__)
#define NML_DBUS_LOG_D(...) NML_DBUS_LOG (NML_DBUS_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define NML_DBUS_LOG_W(...) NML_DBUS_LOG (NML_DBUS_LOG_LEVEL_WARN,  __VA_ARGS__)
#define NML_DBUS_LOG_E(...) NML_DBUS_LOG (NML_DBUS_LOG_LEVEL_ERROR, __VA_ARGS__)

#define NML_NMCLIENT_LOG(level, self, ...) \
	NML_DBUS_LOG ((level), \
	              "nmclient["NM_HASH_OBFUSCATE_PTR_FMT"]: " _NM_UTILS_MACRO_FIRST (__VA_ARGS__), \
	              NM_HASH_OBFUSCATE_PTR (self) \
	              _NM_UTILS_MACRO_REST (__VA_ARGS__))

#define NML_NMCLIENT_LOG_T(self, ...) NML_NMCLIENT_LOG (NML_DBUS_LOG_LEVEL_TRACE, self, __VA_ARGS__)
#define NML_NMCLIENT_LOG_D(self, ...) NML_NMCLIENT_LOG (NML_DBUS_LOG_LEVEL_DEBUG, self, __VA_ARGS__)
#define NML_NMCLIENT_LOG_W(self, ...) NML_NMCLIENT_LOG (NML_DBUS_LOG_LEVEL_WARN,  self, __VA_ARGS__)
#define NML_NMCLIENT_LOG_E(self, ...) NML_NMCLIENT_LOG (NML_DBUS_LOG_LEVEL_ERROR, self, __VA_ARGS__)

/*****************************************************************************/

static inline const char *
_nml_coerce_property_str_not_null (const char *str)
{
	return str ?: "";
}

static inline const char *
_nml_coerce_property_str_not_empty (const char *str)
{
	return str && str[0] ? str : NULL;
}

static inline const char *
_nml_coerce_property_object_path (NMRefString *path)
{
	if (!path)
		return NULL;
	return nm_dbus_path_not_empty (path->str);
}

static inline const char *const*
_nml_coerce_property_strv_not_null (char **strv)
{
	return ((const char *const*) strv) ?: NM_PTRARRAY_EMPTY (const char *);
}

/*****************************************************************************/

typedef struct _NMLDBusObject       NMLDBusObject;
typedef struct _NMLDBusObjWatcher   NMLDBusObjWatcher;
typedef struct _NMLDBusObjPropData  NMLDBusObjPropData;
typedef struct _NMLDBusObjIfaceData NMLDBusObjIfaceData;
typedef struct _NMLDBusMetaIface    NMLDBusMetaIface;

typedef enum {
	NML_DBUS_META_INTERFACE_PRIO_NONE             = 0,
	NML_DBUS_META_INTERFACE_PRIO_NMCLIENT         = 1,
	NML_DBUS_META_INTERFACE_PRIO_PARENT_TYPE      = 2,
	NML_DBUS_META_INTERFACE_PRIO_INSTANTIATE_LOW  = 3,
	NML_DBUS_META_INTERFACE_PRIO_INSTANTIATE_HIGH = 4,
} NMLDBusMetaInteracePrio;

typedef struct _NMLDBusPropertyO  NMLDBusPropertyO;
typedef struct _NMLDBusPropertyAO NMLDBusPropertyAO;

typedef struct {
	GType (*get_o_type_fcn) (void);

	/* This is to untangle loops. Device.ActiveConnection and ActiveConnection.Devices
	 * reference each other. One of them must allow to be considered visible without
	 * the other one. */
	bool ready_without_visible:1;

} NMLDBusPropertVTableO;

typedef struct {
	GType (*get_o_type_fcn) (void);

	void (*notify_changed_ao) (NMLDBusPropertyAO *pr_ao,
	                           NMClient *self,
	                           NMObject *nmobj,
	                           gboolean is_added /* or else removed */);

	gboolean (*check_nmobj_visible_fcn) (GObject *nmobj);

	/* This is to untangle loops. Device.ActiveConnection and ActiveConnection.Devices
	 * reference each other. One of them must allow to be considered visible without
	 * the other one. */
	bool ready_without_visible:1;

} NMLDBusPropertVTableAO;

struct _NMLDBusPropertyO {
	NMLDBusObject *owner_dbobj;
	NMLDBusObjWatcher *obj_watcher;
	GObject *nmobj;
	const NMLDBusMetaIface *meta_iface;
	guint dbus_property_idx;
	bool is_ready:1;
};

gpointer nml_dbus_property_o_get_obj (NMLDBusPropertyO *pr_o);

gboolean nml_dbus_property_o_is_ready (const NMLDBusPropertyO *pr_o);

void nml_dbus_property_o_clear (NMLDBusPropertyO *pr_o,
                                NMClient *client);

struct _NMLDBusPropertyAO {
	CList lst;
	GHashTable *hash;
	NMLDBusObject *owner_dbobj;
	const NMLDBusMetaIface *meta_iface;
	GPtrArray *arr;
	guint dbus_property_idx;
	guint n_not_ready;
};

const GPtrArray *nml_dbus_property_ao_get_objs_as_ptrarray (NMLDBusPropertyAO *pr_ao);

gboolean nml_dbus_property_ao_is_ready (const NMLDBusPropertyAO *pr_ao);

void nml_dbus_property_ao_clear (NMLDBusPropertyAO *pr_ao,
                                 NMClient *client);

typedef enum {
	NML_DBUS_NOTIFY_UPDATE_PROP_FLAGS_NONE   =   0,
	NML_DBUS_NOTIFY_UPDATE_PROP_FLAGS_NOTIFY = 0x1,
} NMLDBusNotifyUpdatePropFlags;

NMLDBusNotifyUpdatePropFlags _nml_dbus_notify_update_prop_ignore (NMClient *client,
                                                                  NMLDBusObject *dbobj,
                                                                  const NMLDBusMetaIface *meta_iface,
                                                                  guint dbus_property_idx,
                                                                  GVariant *value);

NMLDBusNotifyUpdatePropFlags _nml_dbus_notify_update_prop_o (NMClient *client,
                                                             NMLDBusObject *dbobj,
                                                             const NMLDBusMetaIface *meta_iface,
                                                             guint dbus_property_idx,
                                                             GVariant *value);

typedef struct {
	const char *dbus_property_name;
	const GVariantType *dbus_type;

	guint16 prop_struct_offset;

	guint8 obj_properties_idx;

	bool use_notify_update_prop:1;

	bool obj_property_no_reverse_idx:1;

	union {
		union {
			const NMLDBusPropertVTableO *property_vtable_o;
			const NMLDBusPropertVTableAO *property_vtable_ao;
		} extra;

		NMLDBusNotifyUpdatePropFlags (*notify_update_prop) (NMClient *client,
		                                                    NMLDBusObject *dbobj,
		                                                    const NMLDBusMetaIface *meta_iface,
		                                                    guint dbus_property_idx,
		                                                    GVariant *value);
	};

} NMLDBusMetaProperty;

#define NML_DBUS_META_PROPERTY_INIT(v_dbus_property_name, \
                                    v_dbus_type, \
                                    v_obj_properties_idx, \
                                    ...) \
	{ \
		.dbus_property_name = ""v_dbus_property_name"", \
		.dbus_type          = NM_G_VARIANT_TYPE (""v_dbus_type""), \
		.obj_properties_idx = v_obj_properties_idx, \
		##__VA_ARGS__ \
	}

#define _NML_DBUS_META_PROPERTY_INIT_DEFAULT(v_dbus_type, \
                                             v_exp_type, \
                                             v_dbus_property_name, \
                                             v_obj_properties_idx, \
                                             v_container, \
                                             v_field) \
	NML_DBUS_META_PROPERTY_INIT (v_dbus_property_name, \
	                             v_dbus_type, \
	                             v_obj_properties_idx, \
	                             .prop_struct_offset = NM_STRUCT_OFFSET_ENSURE_TYPE (v_exp_type, v_container, v_field))

#define NML_DBUS_META_PROPERTY_INIT_B(...)  _NML_DBUS_META_PROPERTY_INIT_DEFAULT ("b",  bool,     __VA_ARGS__)
#define NML_DBUS_META_PROPERTY_INIT_Y(...)  _NML_DBUS_META_PROPERTY_INIT_DEFAULT ("y",  guint8,   __VA_ARGS__)
#define NML_DBUS_META_PROPERTY_INIT_Q(...)  _NML_DBUS_META_PROPERTY_INIT_DEFAULT ("q",  guint16,  __VA_ARGS__)
#define NML_DBUS_META_PROPERTY_INIT_I(...)  _NML_DBUS_META_PROPERTY_INIT_DEFAULT ("i",  gint32,   __VA_ARGS__)
#define NML_DBUS_META_PROPERTY_INIT_U(...)  _NML_DBUS_META_PROPERTY_INIT_DEFAULT ("u",  guint32,  __VA_ARGS__)
#define NML_DBUS_META_PROPERTY_INIT_X(...)  _NML_DBUS_META_PROPERTY_INIT_DEFAULT ("x",  gint64,   __VA_ARGS__)
#define NML_DBUS_META_PROPERTY_INIT_T(...)  _NML_DBUS_META_PROPERTY_INIT_DEFAULT ("t",  guint64,  __VA_ARGS__)
#define NML_DBUS_META_PROPERTY_INIT_S(...)  _NML_DBUS_META_PROPERTY_INIT_DEFAULT ("s",  char *,   __VA_ARGS__)
#define NML_DBUS_META_PROPERTY_INIT_AS(...) _NML_DBUS_META_PROPERTY_INIT_DEFAULT ("as", char **,  __VA_ARGS__)
#define NML_DBUS_META_PROPERTY_INIT_AY(...) _NML_DBUS_META_PROPERTY_INIT_DEFAULT ("ay", GBytes *, __VA_ARGS__)

#define NML_DBUS_META_PROPERTY_INIT_O(v_dbus_property_name, \
                                      v_obj_properties_idx, \
                                      v_container, \
                                      v_field) \
	NML_DBUS_META_PROPERTY_INIT (v_dbus_property_name, \
	                             "o", \
	                             v_obj_properties_idx, \
	                             .prop_struct_offset = NM_STRUCT_OFFSET_ENSURE_TYPE (NMRefString *, v_container, v_field), \
	                             .use_notify_update_prop = TRUE, \
	                             .notify_update_prop = _nml_dbus_notify_update_prop_o)

#define NML_DBUS_META_PROPERTY_INIT_O_PROP(v_dbus_property_name, \
                                           v_obj_properties_idx, \
                                           v_container, \
                                           v_field, \
                                           v_get_o_type_fcn, \
                                           ...) \
	NML_DBUS_META_PROPERTY_INIT (v_dbus_property_name, \
	                             "o", \
	                             v_obj_properties_idx, \
	                             .prop_struct_offset   = NM_STRUCT_OFFSET_ENSURE_TYPE (NMLDBusPropertyO, v_container, v_field), \
	                             .extra.property_vtable_o = &((const NMLDBusPropertVTableO) { \
	                                 .get_o_type_fcn = (v_get_o_type_fcn), \
	                                 ##__VA_ARGS__ \
	                             }))

#define NML_DBUS_META_PROPERTY_INIT_AO_PROP(v_dbus_property_name, \
                                            v_obj_properties_idx, \
                                            v_container, \
                                            v_field, \
                                            v_get_o_type_fcn, \
                                            ...) \
	NML_DBUS_META_PROPERTY_INIT (v_dbus_property_name, \
	                             "ao", \
	                             v_obj_properties_idx, \
	                             .prop_struct_offset   = NM_STRUCT_OFFSET_ENSURE_TYPE (NMLDBusPropertyAO, v_container, v_field), \
	                             .extra.property_vtable_ao = &((const NMLDBusPropertVTableAO) { \
	                                 .get_o_type_fcn = (v_get_o_type_fcn), \
	                                 ##__VA_ARGS__ \
	                             }))

#define NML_DBUS_META_PROPERTY_INIT_FCN(v_dbus_property_name, \
                                        v_obj_properties_idx, \
                                        v_dbus_type, \
                                        v_notify_update_prop, \
                                        ...) \
	NML_DBUS_META_PROPERTY_INIT (v_dbus_property_name, \
	                             v_dbus_type, \
	                             v_obj_properties_idx, \
	                             .use_notify_update_prop = TRUE, \
	                             .notify_update_prop     = (v_notify_update_prop), \
	                             ##__VA_ARGS__)

#define NML_DBUS_META_PROPERTY_INIT_IGNORE(v_dbus_property_name, \
                                           v_dbus_type) \
	NML_DBUS_META_PROPERTY_INIT (v_dbus_property_name, \
	                             v_dbus_type, \
	                             0, \
	                             .use_notify_update_prop = TRUE, \
	                             .notify_update_prop = _nml_dbus_notify_update_prop_ignore)

/* "TODO" is like "IGNORE". The difference is that we don't plan to ever implement "IGNORE", but
 * "TODO" is something we should add support for. */
#define NML_DBUS_META_PROPERTY_INIT_TODO(...) \
	NML_DBUS_META_PROPERTY_INIT_IGNORE (__VA_ARGS__)

typedef struct _NMLDBusMetaIface {
	const char *dbus_iface_name;
	GType (*get_type_fcn) (void);
	const GParamSpec *const*obj_properties;
	const NMLDBusMetaProperty *dbus_properties;
	const guint8 *obj_properties_reverse_idx;
	guint8 n_dbus_properties;
	guint8 n_obj_properties;

	/* The offsets in NMLDBusMetaProperty are based on some base struct.
	 * If this is 0, then the base struct is the GObject pointer itself.
	 * If this is non-null, then we expect at that location a pointer
	 * to the offset. */
	guint8 base_struct_offset;

	NMLDBusMetaInteracePrio interface_prio:3;
} NMLDBusMetaIface;

#define NML_DBUS_META_IFACE_OBJ_PROPERTIES() \
	.obj_properties             = (const GParamSpec *const*) (obj_properties), \
	.n_obj_properties           = _PROPERTY_ENUMS_LAST, \
	.obj_properties_reverse_idx = ((guint8 [_PROPERTY_ENUMS_LAST]) { })

#define NML_DBUS_META_IFACE_DBUS_PROPERTIES(...) \
	.dbus_properties            = ((const NMLDBusMetaProperty []) { __VA_ARGS__ }), \
	.n_dbus_properties          = sizeof ((const NMLDBusMetaProperty []) { __VA_ARGS__ }) / sizeof (NMLDBusMetaProperty) \

#define NML_DBUS_META_IFACE_INIT(v_dbus_iface_name, \
                                 v_get_type_fcn, \
                                 v_interface_prio, \
                                 ...) \
	{ \
		.dbus_iface_name            = ""v_dbus_iface_name"", \
		.get_type_fcn               = v_get_type_fcn, \
		.interface_prio             = v_interface_prio, \
		##__VA_ARGS__ \
	}

#define NML_DBUS_META_IFACE_INIT_PROP(...) \
	NML_DBUS_META_IFACE_INIT (__VA_ARGS__ \
	                          NML_DBUS_META_IFACE_OBJ_PROPERTIES ())

extern const NMLDBusMetaIface *const _nml_dbus_meta_ifaces[43];

extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_accesspoint;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_agentmanager;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_checkpoint;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_connection_active;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_adsl;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_bluetooth;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_bond;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_bridge;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_dummy;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_generic;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_infiniband;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_iptunnel;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_lowpan;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_macsec;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_macvlan;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_modem;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_olpcmesh;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_ovsbridge;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_ovsinterface;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_ovsport;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_ppp;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_statistics;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_team;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_tun;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_veth;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_vlan;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_vxlan;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_wifip2p;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_wired;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_wireguard;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_wireless;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_device_wpan;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_dhcp4config;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_dhcp6config;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_dnsmanager;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_ip4config;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_ip6config;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_settings;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_settings_connection;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_vpn_connection;
extern const NMLDBusMetaIface _nml_dbus_meta_iface_nm_wifip2ppeer;

const NMLDBusMetaIface *nml_dbus_meta_iface_get (const char *dbus_iface_name);

const NMLDBusMetaProperty *nml_dbus_meta_property_get (const NMLDBusMetaIface *meta_iface,
                                                       const char *dbus_property_name,
                                                       guint *out_idx);

void _nml_dbus_meta_class_init_with_properties_impl (GObjectClass *object_class, const NMLDBusMetaIface *const*meta_iface);
#define _nml_dbus_meta_class_init_with_properties(object_class, ...) \
	_nml_dbus_meta_class_init_with_properties_impl ((object_class), ((const NMLDBusMetaIface *const[]) { __VA_ARGS__, NULL }))

/*****************************************************************************/

typedef enum {
	NML_DBUS_OBJ_STATE_UNLINKED     = 0,
	NML_DBUS_OBJ_STATE_WATCHED_ONLY,
	NML_DBUS_OBJ_STATE_ON_DBUS,
	NML_DBUS_OBJ_STATE_WITH_NMOBJ_NOT_READY,
	NML_DBUS_OBJ_STATE_WITH_NMOBJ_MAYBE_READY,
	NML_DBUS_OBJ_STATE_WITH_NMOBJ_VISIBLE,
	NML_DBUS_OBJ_STATE_WITH_NMOBJ_HIDDEN,
} NMLDBusObjState;

struct _NMLDBusObject {
	NMRefString *dbus_path;

	CList iface_lst_head;
	CList removed_iface_lst_head;
	CList changed_obj_lst;

	CList watcher_lst_head;

	CList notify_watchers_lst;

	CList dbus_objects_lst;

	GObject *nmobj;
	int ref_count;

	NMLDBusObjState obj_state:4;
};

static inline gboolean
NML_IS_DBUS_OBJECT (NMLDBusObject *dbobj)
{
	nm_assert (   !dbobj
	           || (   NM_IS_REF_STRING (dbobj->dbus_path)
	               && dbobj->ref_count > 0));
	nm_assert (   !dbobj->nmobj
	           || NM_IS_OBJECT (dbobj->nmobj)
	           || NM_IS_CLIENT (dbobj->nmobj));
	return !!dbobj;
}

NMLDBusObject *nml_dbus_object_ref (NMLDBusObject *dbobj);

void nml_dbus_object_unref (NMLDBusObject *dbobj);

NM_AUTO_DEFINE_FCN0 (NMLDBusObject *, _nm_auto_unref_nml_dbusobj, nml_dbus_object_unref)
#define nm_auto_unref_nml_dbusobj nm_auto (_nm_auto_unref_nml_dbusobj)

NMLDBusObjIfaceData *nml_dbus_object_iface_data_get (NMLDBusObject *dbobj,
                                                     const char *dbus_iface_name,
                                                     gboolean allow_create);

gpointer nml_dbus_object_get_property_location (NMLDBusObject *dbobj,
                                                const NMLDBusMetaIface *meta_iface,
                                                const NMLDBusMetaProperty *meta_property);

/*****************************************************************************/

/* NMClient is not an NMObject, but in some aspects we want to track it like
 * an NMObject. For that, both NMClient and NMObject "implement" NMObjectBase,
 * despite not actually implementing such a GObject type. */
typedef struct {
	GObject parent;
	CList queue_notify_lst;
	bool is_disposing:1;
} NMObjectBase;

typedef struct {
	GObjectClass parent;
} NMObjectBaseClass;

struct _NMObjectPrivate;

struct _NMObject {
	union {
		GObject parent;
		NMObjectBase obj_base;
	};
	struct _NMObjectPrivate *_priv;
};

typedef struct _NMObjectClassFieldInfo {
	const struct _NMObjectClassFieldInfo *parent;
	NMObjectClass *klass;
	guint16 offset;
	guint16 num;
} _NMObjectClassFieldInfo;

struct _NMObjectClass {
	union {
		GObjectClass parent;
		NMObjectBaseClass obj_base;
	};

	void (*register_client) (NMObject *self,
	                         NMClient *client,
	                         NMLDBusObject *dbobj);

	void (*unregister_client) (NMObject *self,
	                           NMClient *client,
	                           NMLDBusObject *dbobj);

	NMLDBusObjState (*is_visible) (NMObject *self);

	const _NMObjectClassFieldInfo *property_o_info;
	const _NMObjectClassFieldInfo *property_ao_info;

	guint16 priv_ptr_offset;

	bool priv_ptr_indirect:1;
};

#define _NM_OBJECT_CLASS_INIT_PRIV_PTR_DIRECT(nm_object_class, type_name) \
	G_STMT_START { \
		(nm_object_class)->priv_ptr_offset = NM_STRUCT_OFFSET_ENSURE_TYPE (type_name##Private, type_name, _priv); \
		(nm_object_class)->priv_ptr_indirect = FALSE; \
	} G_STMT_END

#define _NM_OBJECT_CLASS_INIT_PRIV_PTR_INDIRECT(nm_object_class, type_name) \
	G_STMT_START { \
		(nm_object_class)->priv_ptr_offset = NM_STRUCT_OFFSET_ENSURE_TYPE (type_name##Private *, type_name, _priv); \
		(nm_object_class)->priv_ptr_indirect = TRUE; \
	} G_STMT_END

#define _NM_OBJECT_CLASS_INIT_FIELD_INFO(_nm_object_class, _field_name, _offset, _num) \
	G_STMT_START { \
		(_nm_object_class)->_field_name = ({ \
			static _NMObjectClassFieldInfo _f; \
			\
			_f = (_NMObjectClassFieldInfo) { \
				.parent = (_nm_object_class)->_field_name, \
				.klass  = (_nm_object_class), \
				.offset = _offset, \
				.num    = _num, \
			}; \
			&_f; \
		}); \
	} G_STMT_END

#define _NM_OBJECT_CLASS_INIT_PROPERTY_O_FIELDS_1(nm_object_class, type_name, field_name) \
	_NM_OBJECT_CLASS_INIT_FIELD_INFO (nm_object_class, \
	                                  property_o_info, \
	                                  NM_STRUCT_OFFSET_ENSURE_TYPE (NMLDBusPropertyO, type_name, field_name), \
	                                  1)

#define _NM_OBJECT_CLASS_INIT_PROPERTY_O_FIELDS_N(nm_object_class, type_name, field_name) \
	_NM_OBJECT_CLASS_INIT_FIELD_INFO (nm_object_class, \
	                                  property_o_info, \
	                                  NM_STRUCT_OFFSET_ENSURE_TYPE (NMLDBusPropertyO *, type_name, field_name), \
	                                  G_N_ELEMENTS (((type_name *) NULL)->field_name))

#define _NM_OBJECT_CLASS_INIT_PROPERTY_AO_FIELDS_1(nm_object_class, type_name, field_name) \
	_NM_OBJECT_CLASS_INIT_FIELD_INFO (nm_object_class, \
	                                  property_ao_info, \
	                                  NM_STRUCT_OFFSET_ENSURE_TYPE (NMLDBusPropertyAO, type_name, field_name), \
	                                  1)

#define _NM_OBJECT_CLASS_INIT_PROPERTY_AO_FIELDS_N(nm_object_class, type_name, field_name) \
	_NM_OBJECT_CLASS_INIT_FIELD_INFO (nm_object_class, \
	                                  property_ao_info, \
	                                  NM_STRUCT_OFFSET_ENSURE_TYPE (NMLDBusPropertyAO *, type_name, field_name), \
	                                  G_N_ELEMENTS (((type_name *) NULL)->field_name))


/*****************************************************************************/

struct _NMDevicePrivate;

struct _NMDevice {
	NMObject parent;
	struct _NMDevicePrivate *_priv;
};

struct _NMDeviceClass {
	struct _NMObjectClass parent;

	gboolean (*connection_compatible) (NMDevice *device,
	                                   NMConnection *connection,
	                                   GError **error);

	const char *(*get_type_description) (NMDevice *device);

	const char *(*get_hw_address) (NMDevice *device);

	GType (*get_setting_type) (NMDevice *device);
};

/*****************************************************************************/

struct _NMActiveConnectionPrivate;

struct _NMActiveConnection {
	NMObject parent;
	struct _NMActiveConnectionPrivate *_priv;
};

struct _NMActiveConnectionClass {
	struct _NMObjectClass parent;
};

/*****************************************************************************/

struct _NMDhcpConfigPrivate;

struct _NMDhcpConfig {
	NMObject parent;
	struct _NMDhcpConfigPrivate *_priv;
};

struct _NMDhcpConfigClass {
	struct _NMObjectClass parent;
};

/*****************************************************************************/

struct _NMIPConfigPrivate;

struct _NMIPConfig {
	NMObject parent;
	struct _NMIPConfigPrivate *_priv;
};

struct _NMIPConfigClass {
	struct _NMObjectClass parent;
};

/*****************************************************************************/

NMLDBusObject *_nm_object_get_dbobj (gpointer self);

const char *_nm_object_get_path (gpointer self);

NMClient *_nm_object_get_client (gpointer self);

GDBusConnection *_nm_client_get_dbus_connection (NMClient *client);

const char *_nm_client_get_dbus_name_owner (NMClient *client);

GMainContext *_nm_client_get_context_main (NMClient *client);
GMainContext *_nm_client_get_context_dbus (NMClient *client);

void _nm_client_queue_notify_object (NMClient *client,
                                     gpointer nmobj,
                                     const GParamSpec *pspec);

void _nm_client_notify_object_changed (NMClient *self,
                                       NMLDBusObject *dbobj);

struct udev *_nm_client_get_udev (NMClient *self);

/*****************************************************************************/

#define NM_CLIENT_NOTIFY_EVENT_PRIO_BEFORE (-100)
#define NM_CLIENT_NOTIFY_EVENT_PRIO_GPROP      0
#define NM_CLIENT_NOTIFY_EVENT_PRIO_AFTER    100

typedef struct _NMClientNotifyEvent        NMClientNotifyEvent;

typedef void (*NMClientNotifyEventCb) (NMClient *self,
                                       gpointer notify_event);

struct _NMClientNotifyEvent {
	CList lst;
	NMClientNotifyEventCb callback;
	int priority;
};

gpointer _nm_client_notify_event_queue (NMClient *self,
                                        int priority,
                                        NMClientNotifyEventCb callback,
                                        gsize event_size);

typedef struct _NMClientNotifyEventWithPtr NMClientNotifyEventWithPtr;

typedef void (*NMClientNotifyEventWithPtrCb) (NMClient *self,
                                              NMClientNotifyEventWithPtr *notify_event);

struct _NMClientNotifyEventWithPtr {
	NMClientNotifyEvent parent;
	gpointer user_data;
};

NMClientNotifyEventWithPtr *_nm_client_notify_event_queue_with_ptr (NMClient *self,
                                                                    int priority,
                                                                    NMClientNotifyEventWithPtrCb callback,
                                                                    gpointer user_data);

void _nm_client_notify_event_queue_emit_obj_signal (NMClient *self,
                                                    GObject *source,
                                                    NMObject *nmobj,
                                                    gboolean is_added /* or else removed */,
                                                    int prio_offset,
                                                    guint signal_id);

/*****************************************************************************/

GError *_nm_client_new_error_nm_not_running (void);
GError *_nm_client_new_error_nm_not_cached (void);

void _nm_client_dbus_call_simple (NMClient *self,
                                  GCancellable *cancellable,
                                  const char *object_path,
                                  const char *interface_name,
                                  const char *method_name,
                                  GVariant *parameters,
                                  const GVariantType *reply_type,
                                  GDBusCallFlags flags,
                                  int timeout_msec,
                                  GAsyncReadyCallback callback,
                                  gpointer user_data);

void _nm_client_dbus_call (NMClient *self,
                           gpointer source_obj,
                           gpointer source_tag,
                           GCancellable *cancellable,
                           GAsyncReadyCallback user_callback,
                           gpointer user_callback_data,
                           const char *object_path,
                           const char *interface_name,
                           const char *method_name,
                           GVariant *parameters,
                           const GVariantType *reply_type,
                           GDBusCallFlags flags,
                           int timeout_msec,
                           GAsyncReadyCallback internal_callback);

GVariant *_nm_client_dbus_call_sync (NMClient *self,
                                     GCancellable *cancellable,
                                     const char *object_path,
                                     const char *interface_name,
                                     const char *method_name,
                                     GVariant *parameters,
                                     const GVariantType *reply_type,
                                     GDBusCallFlags flags,
                                     int timeout_msec,
                                     gboolean strip_dbus_error,
                                     GError **error);

gboolean _nm_client_dbus_call_sync_void (NMClient *self,
                                         GCancellable *cancellable,
                                         const char *object_path,
                                         const char *interface_name,
                                         const char *method_name,
                                         GVariant *parameters,
                                         GDBusCallFlags flags,
                                         int timeout_msec,
                                         gboolean strip_dbus_error,
                                         GError **error);

void _nm_client_set_property_sync_legacy (NMClient *self,
                                          const char *object_path,
                                          const char *interface,
                                          const char *prop_name,
                                          const char *format_string,
                                          ...);

/*****************************************************************************/

void _nm_client_get_settings_call (NMClient *self,
                                   NMLDBusObject *dbobj);

GCancellable *_nm_remote_settings_get_settings_prepare (NMRemoteConnection *self);

void _nm_remote_settings_get_settings_commit (NMRemoteConnection *self,
                                              GVariant *settings);

/*****************************************************************************/

void _nm_active_connection_state_changed_commit (NMActiveConnection *self,
                                                 guint32 state,
                                                 guint32 reason);

void _nm_vpn_connection_state_changed_commit (NMVpnConnection *self,
                                              guint32 state,
                                              guint32 reason);

/*****************************************************************************/

#endif /* __NM_LIBNM_UTILS_H__ */
