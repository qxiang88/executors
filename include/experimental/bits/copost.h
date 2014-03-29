//
// copost.h
// ~~~~~~~~
// Schedule functions to run concurrently later.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXECUTORS_EXPERIMENTAL_BITS_COPOST_H
#define EXECUTORS_EXPERIMENTAL_BITS_COPOST_H

#include <experimental/bits/coinvoker.h>

namespace std {
namespace experimental {

template <class _Func1, class _Func2, class _CompletionToken>
auto copost(_Func1&& __f1, _Func2&& __f2, _CompletionToken&& __token)
{
  typedef continuation_traits<_Func1> _Traits1;
  typedef typename _Traits1::signature _Signature1;

  typedef continuation_traits<_Func2> _Traits2;
  typedef typename _Traits2::signature _Signature2;

  typedef __signature_cat_t<_Signature1, _Signature2> _HandlerSignature;
  typedef handler_type_t<_CompletionToken, _HandlerSignature> _Handler;

  async_completion<_CompletionToken, _HandlerSignature> __completion(__token);

  unique_ptr<__coinvoker_handler<_Handler, _Signature1, _Signature2>> __h(
    new __coinvoker_handler<_Handler, _Signature1, _Signature2>(std::move(__completion.handler)));

  auto __i1(_Traits1::chain(forward<_Func1>(__f1), __coinvoker<0, _Handler, _Signature1, _Signature2>(__h.get())));
  auto __i2(_Traits2::chain(forward<_Func2>(__f2), __coinvoker<1, _Handler, _Signature1, _Signature2>(__h.get())));

  __h->_Prime();
  __h.release();

  (post)(std::move(__i1));
  (post)(std::move(__i2));

  return __completion.result.get();
}

template <class _Executor, class _Func1, class _Func2, class _CompletionToken>
auto copost(_Executor&& __e, _Func1&& __f1, _Func2&& __f2, _CompletionToken&& __token)
{
  return (copost)(__e.wrap(forward<_Func1>(__f1)),
    __e.wrap(forward<_Func2>(__f2)), forward<_CompletionToken>(__token));
}

} // namespace experimental
} // namespace std

#endif