//
// Created by Chad Hurst on 3/3/20.
//

#pragma once

#include <iostream>

#if 0
# define NEW(T) new T
# define PTR(T) T*
# define CAST(T) dynamic_cast<T*>
# define THIS this
# define ENABLE_THIS(T) /* empty */

#else
# define NEW(T) std::make_shared<T>
# define PTR(T) std::shared_ptr<T>
# define CAST(T) std::dynamic_pointer_cast<T>
# define THIS std::shared_from_this()
# define ENABLE_THIS(T) : public std::enable_shared_from_this<T>

#endif




