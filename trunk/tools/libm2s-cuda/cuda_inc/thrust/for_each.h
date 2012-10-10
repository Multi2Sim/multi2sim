/*
 *  Copyright 2008-2011 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


/*! \file for_each.h
 *  \brief Defines the interface to for_each.
 */

#pragma once

#include <thrust/detail/config.h>

namespace thrust
{

/*! \addtogroup modifying
 *  \ingroup transformations
 *  \{
 */

/*! \p for_each applies the function object \p f to each element
 *  in the range <tt>[first, last)</tt>; \p f's return value, if any,
 *  is ignored. Unlike the C++ Standard Template Library function
 *  <tt>std::for_each</tt>, this version offers no guarantee on
 *  order of execution. For this reason, this version of \p for_each
 *  has no return value.
 *
 *  \param first The beginning of the sequence.
 *  \param last The end of the sequence.
 *  \param f The function object to apply to the range <tt>[first, last)</tt>.
 *
 *  \tparam InputIterator is a model of <a href="http://www.sgi.com/tech/stl/InputIterator">Input Iterator</a>,
 *          and \p InputIterator's \c value_type is convertible to \p UnaryFunction's \c argument_type.
 *  \tparam UnaryFunction is a model of <a href="http://www.sgi.com/tech/stl/UnaryFunction">Unary Function</a>,
 *          and \p UnaryFunction does not apply any non-constant operation through its argument.
 *
 *  The following code snippet demonstrates how to use \p for_each to print the elements
 *  of a \p device_vector.
 *
 *  \code
 *  #include <thrust/for_each.h>
 *  #include <thrust/device_vector.h>
 *  #include <stdio.h>
 *
 *  struct printf_functor
 *  {
 *    __host__ __device__
 *    void operator()(int x)
 *    {
 *      // note that using printf in a __device__ function requires
 *      // code compiled for a GPU with compute capability 2.0 or
 *      // higher (nvcc --arch=sm_20)
 *      printf("%d\n");
 *    }
 *  };
 *  ...
 *  thrust::device_vector<int> d_vec(3);
 *  d_vec[0] = 0; d_vec[1] = 1; d_vec[2] = 2;
 *
 *  thrust::for_each(d_vec.begin(), d_vec.end(), printf_functor());
 *
 *  // 0 1 2 is printed to standard output in some unspecified order
 *  \endcode
 *
 *  \see http://www.sgi.com/tech/stl/for_each.html
 */
template<typename InputIterator,
         typename UnaryFunction>
void for_each(InputIterator first,
              InputIterator last,
              UnaryFunction f);

/*! \} // end modifying
 */

} // end namespace thrust

#include <thrust/detail/for_each.inl>

