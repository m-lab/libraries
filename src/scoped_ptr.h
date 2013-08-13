// Copyright 2012 M-Lab. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _MLAB_SCOPED_PTR_H_
#define _MLAB_SCOPED_PTR_H_

namespace mlab {

template<typename T>
class scoped_ptr {
 public:
  explicit scoped_ptr(T* ptr) : ptr_(ptr) { }
  ~scoped_ptr() { delete ptr_; }

  T* operator->() { return ptr_; }
  const T* operator->() const { return ptr_; }

  T* get() { return ptr_; }
  const T* get() const { return ptr_; }

 private:
  T* ptr_;
};

}  // namespace mlab

#endif  // _MLAB_SCOPED_PTR_H_
