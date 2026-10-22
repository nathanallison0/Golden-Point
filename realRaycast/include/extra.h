#ifndef _GPOINT_EXTRA_H_
#define _GPOINT_EXTRA_H_

#define extra(type, attrs) typedef struct {attrs} type##_extra
#define __def_extra_var(type, var) type##_extra *extra = (type##_extra *) var->extra
#define __def_extra(type) __def_extra_var(type, type)

#endif