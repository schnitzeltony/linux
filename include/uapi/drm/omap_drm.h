/*
 * include/uapi/drm/omap_drm.h
 *
 * Copyright (C) 2011 Texas Instruments
 * Author: Rob Clark <rob@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __OMAP_DRM_H__
#define __OMAP_DRM_H__

#include <drm/drm.h>

/* Please note that modifications to all structs defined here are
 * subject to backwards-compatibility constraints.
 */

#define OMAP_PARAM_CHIPSET_ID	1	/* ie. 0x3430, 0x4430, etc */

struct drm_omap_param {
	uint64_t param;			/* in */
	uint64_t value;			/* in (set_param), out (get_param) */
};

struct drm_omap_get_base {
	char plugin_name[64];           /* in */
	uint32_t ioctl_base;            /* out */
	uint32_t __pad;
};

#define OMAP_BO_SCANOUT		0x00000001	/* scanout capable (phys contiguous) */
#define OMAP_BO_CACHE_MASK	0x00000006	/* cache type mask, see cache modes */
#define OMAP_BO_TILED_MASK	0x00000f00	/* tiled mapping mask, see tiled modes */

/* cache modes */
#define OMAP_BO_CACHED		0x00000000	/* default */
#define OMAP_BO_WC		0x00000002	/* write-combine */
#define OMAP_BO_UNCACHED	0x00000004	/* strongly-ordered (uncached) */

/* tiled modes */
#define OMAP_BO_TILED_8		0x00000100
#define OMAP_BO_TILED_16	0x00000200
#define OMAP_BO_TILED_32	0x00000300
#define OMAP_BO_TILED		(OMAP_BO_TILED_8 | OMAP_BO_TILED_16 | OMAP_BO_TILED_32)

union omap_gem_size {
	uint32_t bytes;		/* (for non-tiled formats) */
	struct {
		uint16_t width;
		uint16_t height;
	} tiled;		/* (for tiled formats) */
};

struct drm_omap_gem_new {
	union omap_gem_size size;	/* in */
	uint32_t flags;			/* in */
	uint32_t handle;		/* out */
	uint32_t __pad;
};

/* mask of operations: */
enum omap_gem_op {
	OMAP_GEM_READ = 0x01,
	OMAP_GEM_WRITE = 0x02,
};

struct drm_omap_gem_cpu_prep {
	uint32_t handle;		/* buffer handle (in) */
	uint32_t op;			/* mask of omap_gem_op (in) */
};

struct drm_omap_gem_cpu_fini {
	uint32_t handle;		/* buffer handle (in) */
	uint32_t op;			/* mask of omap_gem_op (in) */
	/* TODO maybe here we pass down info about what regions are touched
	 * by sw so we can be clever about cache ops?  For now a placeholder,
	 * set to zero and we just do full buffer flush..
	 */
	uint32_t nregions;
	uint32_t __pad;
};

struct drm_omap_gem_info {
	uint32_t handle;		/* buffer handle (in) */
	uint32_t pad;
	uint64_t offset;		/* mmap offset (out) */
	/* note: in case of tiled buffers, the user virtual size can be
	 * different from the physical size (ie. how many pages are needed
	 * to back the object) which is returned in DRM_IOCTL_GEM_OPEN..
	 * This size here is the one that should be used if you want to
	 * mmap() the buffer:
	 */
	uint32_t size;			/* virtual size for mmap'ing (out) */
	uint32_t __pad;
};

#define DRM_OMAP_GET_PARAM		0x00
#define DRM_OMAP_SET_PARAM		0x01
#define DRM_OMAP_GET_BASE		0x02
#define DRM_OMAP_GEM_NEW		0x03
#define DRM_OMAP_GEM_CPU_PREP		0x04
#define DRM_OMAP_GEM_CPU_FINI		0x05
#define DRM_OMAP_GEM_INFO		0x06
#define DRM_OMAP_NUM_IOCTLS		0x07

#define DRM_IOCTL_OMAP_GET_PARAM	DRM_IOWR(DRM_COMMAND_BASE + DRM_OMAP_GET_PARAM, struct drm_omap_param)
#define DRM_IOCTL_OMAP_SET_PARAM	DRM_IOW (DRM_COMMAND_BASE + DRM_OMAP_SET_PARAM, struct drm_omap_param)
#define DRM_IOCTL_OMAP_GET_BASE		DRM_IOWR(DRM_COMMAND_BASE + DRM_OMAP_GET_BASE, struct drm_omap_get_base)
#define DRM_IOCTL_OMAP_GEM_NEW		DRM_IOWR(DRM_COMMAND_BASE + DRM_OMAP_GEM_NEW, struct drm_omap_gem_new)
#define DRM_IOCTL_OMAP_GEM_CPU_PREP	DRM_IOW (DRM_COMMAND_BASE + DRM_OMAP_GEM_CPU_PREP, struct drm_omap_gem_cpu_prep)
#define DRM_IOCTL_OMAP_GEM_CPU_FINI	DRM_IOW (DRM_COMMAND_BASE + DRM_OMAP_GEM_CPU_FINI, struct drm_omap_gem_cpu_fini)
#define DRM_IOCTL_OMAP_GEM_INFO		DRM_IOWR(DRM_COMMAND_BASE + DRM_OMAP_GEM_INFO, struct drm_omap_gem_info)


/*
 * pvr door below
 */
void * omap_drm_file_priv(struct drm_file *file, int mapper_id);
void omap_drm_file_set_priv(struct drm_file *file, int mapper_id, void *priv);

/* interface that plug-in drivers can implement */
struct omap_drm_plugin {
	const char *name;

	/* drm functions */
	int (*open)(struct drm_device *dev, struct drm_file *file);
	int (*load)(struct drm_device *dev, unsigned long flags);
	int (*unload)(struct drm_device *dev);
	int (*release)(struct drm_device *dev, struct drm_file *file);

	struct drm_ioctl_desc *ioctls;
	int num_ioctls;
	int ioctl_base;

	struct list_head list;  /* note, this means struct can't be const.. */
};

int omap_drm_register_plugin(struct omap_drm_plugin *plugin);
int omap_drm_unregister_plugin(struct omap_drm_plugin *plugin);

int omap_drm_register_mapper(void);
void omap_drm_unregister_mapper(int id);

/* external mappers should get paddr or pages when it needs the pages pinned
 * and put when done..
 */
int omap_gem_get_paddr(struct drm_gem_object *obj, dma_addr_t *paddr, bool remap);
int omap_gem_put_paddr(struct drm_gem_object *obj);
int omap_gem_get_pages(struct drm_gem_object *obj, struct page ***pages, bool remap);
int omap_gem_put_pages(struct drm_gem_object *obj);

uint32_t omap_gem_flags(struct drm_gem_object *obj);
void * omap_gem_priv(struct drm_gem_object *obj, int mapper_id);
void omap_gem_set_priv(struct drm_gem_object *obj, int mapper_id, void *priv);
uint64_t omap_gem_mmap_offset(struct drm_gem_object *obj);

/* for external plugin buffers wrapped as GEM object (via. omap_gem_new_ext())
 * a vm_ops struct can be provided to get callback notification of various
 * events..
 */
struct omap_gem_vm_ops {
	void (*open)(struct vm_area_struct * area);
	void (*close)(struct vm_area_struct * area);

	/* note: mmap is not expected to do anything.. it is just to allow buffer
	 * allocate to update it's own internal state
	 */
	void (*mmap)(struct file *, struct vm_area_struct *);
};

struct drm_gem_object * omap_gem_new_ext(struct drm_device *dev,
		union omap_gem_size gsize, uint32_t flags, dma_addr_t paddr, struct page **pages,
		struct omap_gem_vm_ops *ops);
struct drm_gem_object *omap_gem_new(struct drm_device *dev,
		union omap_gem_size gsize, uint32_t flags);

void omap_gem_op_update(void);
int omap_gem_op_start(struct drm_gem_object *obj, enum omap_gem_op op);
int omap_gem_op_finish(struct drm_gem_object *obj, enum omap_gem_op op);
int omap_gem_op_sync(struct drm_gem_object *obj, enum omap_gem_op op);
int omap_gem_op_async(struct drm_gem_object *obj, enum omap_gem_op op,
        void (*fxn)(void *arg), void *arg);
int omap_gem_set_sync_object(struct drm_gem_object *obj, void *syncobj);


#endif /* __OMAP_DRM_H__ */
