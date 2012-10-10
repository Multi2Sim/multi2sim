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

#include <thrust/detail/config.h>

#if THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_NVCC

#include <thrust/iterator/iterator_traits.h>
#include <thrust/pair.h>
#include <thrust/detail/device/cuda/block/set_difference.h>
#include <thrust/detail/device/cuda/detail/split_for_set_operation.h>
#include <thrust/detail/device/cuda/detail/set_operation.h>

namespace thrust
{
namespace detail
{
namespace device
{
namespace cuda
{

namespace set_difference_detail
{

struct block_convergent_set_difference_functor
{
  __host__ __device__ __forceinline__
  static size_t get_min_size_of_result_in_number_of_elements(size_t size_of_range1,
                                                             size_t size_of_range2)
  {
    // set_difference could result in 0 output
    return 0u;
  }

  __host__ __device__ __forceinline__
  static size_t get_max_size_of_result_in_number_of_elements(size_t size_of_range1,
                                                             size_t size_of_range2)
  {
    // set_difference could output all of range1
    return size_of_range1;
  }

  __host__ __device__ __forceinline__
  static unsigned int get_temporary_array_size_in_number_of_bytes(unsigned int block_size)
  {
    return block_size * sizeof(int);
  }

  // operator() simply calls the block-wise function
  template<typename RandomAccessIterator1,
           typename RandomAccessIterator2,
           typename RandomAccessIterator3,
           typename StrictWeakOrdering>
  __device__ __forceinline__
    RandomAccessIterator3 operator()(RandomAccessIterator1 first1,
                                     RandomAccessIterator1 last1,
                                     RandomAccessIterator2 first2,
                                     RandomAccessIterator2 last2,
                                     void *temporary,
                                     RandomAccessIterator3 result,
                                     StrictWeakOrdering comp)
  {
    return block::set_difference(first1,last1,first2,last2,reinterpret_cast<int*>(temporary),result,comp);
  } // end operator()()
}; // end block_convergent_set_difference_functor

} // end namespace set_difference_detail


template<typename RandomAccessIterator1,
         typename RandomAccessIterator2, 
	 typename RandomAccessIterator3,
         typename Compare>
RandomAccessIterator3 set_difference(RandomAccessIterator1 first1,
                                     RandomAccessIterator1 last1,
                                     RandomAccessIterator2 first2,
                                     RandomAccessIterator2 last2,
                                     RandomAccessIterator3 result,
                                     Compare comp)
{
  typedef typename thrust::iterator_difference<RandomAccessIterator1>::type difference1;
  typedef typename thrust::iterator_difference<RandomAccessIterator2>::type difference2;

  const difference1 num_elements1 = last1 - first1;

  // check for trivial problem
  if(num_elements1 == 0)
    return result;

  return detail::set_operation(first1, last1,
                               first2, last2,
                               result,
                               comp,
                               detail::split_for_set_operation(),
                               set_difference_detail::block_convergent_set_difference_functor());
} // end set_difference

} // end namespace cuda
} // end namespace device
} // end namespace detail
} // end namespace thrust

#endif // THRUST_DEVICE_COMPILER == THRUST_DEVICE_COMPILER_NVCC

