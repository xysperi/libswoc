.. Licensed to the Apache Software Foundation (ASF) under one
   or more contributor license agreements. See the NOTICE file distributed with this work for
   additional information regarding copyright ownership. The ASF licenses this file to you under the
   Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
   the License. You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software distributed under the License
   is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
   or implied. See the License for the specific language governing permissions and limitations under
   the License.

.. include:: ../common-defs.rst

.. _swoc-intrusive-list:
.. highlight:: cpp
.. default-domain:: cpp

**************
IntrusiveDList
**************

:code:`IntrusiveDList` is a class that provides a double linked list using pointers embeded in the
object. :code:`IntrusiveDList` also acts as a queue. No memory management is done - objects can be
added to and removed from the list but the allocation and deallocation of the objects must be
handled outside the class. This class supports an STL compliant bidirectional iteration. The
iterators automatically convert to pointer as in normal use of this class the contained elements
will be referenced by pointers.

Definition
**********

.. class:: template < typename L > IntrusiveDList

   :libswoc:`Reference documentation <IntrusiveDList>`.

Usage
*****

An instance of :code:`IntrusiveDList` acts as a container for items, maintaining a doubly linked
list / queue of the objects and tracking the number of objects in the container. There are methods
for appending, prepending, and inserting (both before and after a specific element already in the
list). Some care must be taken because it is too expensive to check for an element already being in
the list or in another list. The internal links are set to :code:`nullptr`, therefore one simple check
for being in a list is if either internal link is not :code:`nullptr`. This requires initializing the
internal links to :code:`nullptr`.

There are several uses cases where this data structure is useful.

Explicit object life cycle management is already required.
   In many cases, particularly in Traffic Server, it is a requirement for other reasons to explicit
   manage object life times, through :code:`new` / :code:`delete`. This is this major drawback to
   :code:`IntrusiveDList`, therefore if it's already a requirement using :code:`IntrusiveDList`
   is a good choice because it lets the code do its object management and use an STL compliant
   container without the complexity adding additional helper classes.

Memory Arena
   When using a memory arena for memory management, it is a considerable advantage to have as much
   as possible in the arena. Using standard containers presents the problem of convincing those
   containers to use the arena and interact with it in a non-breaking manner. In contrast adding
   :code:`IntrusiveDList` is simple - construct the base instance in the arena. This avoids any
   clean up of the container, which is the basic reason for using an arena.

Multi-container
   If an object needs to be in multiple containers at the same time, an instrusive container is very
   handy. In practice when this has been used there is a primary container which is a standard STL
   type container which manages the objects. Additional containers are put on top of this by adding
   instrusive links. This requires that all of the containers have the same lifetime and an object
   is in the primary container if it is in any of the intrusive ones. A good example is having an
   least recently used (LRU) list with a standard map container when bouding the number of elements
   in the map. Elements can be put on the head of the LRU list and then, when objects need to be
   removed, pulled from the tail of the LRU list and dropped from that and the map.

Examples
========

In this example the goal is to have a list of :code:`Message` objects. First the class is declared
along with the internal linkage support.

.. literalinclude:: ../../unit_tests/ex_IntrusiveDList.cc
   :lines: 37-62

The struct :code:`Linkage` is used both to provide the descriptor to :code:`IntrusiveDList` and to
contain the link pointers. This isn't necessary - the links could have been direct members
and the implementation of the link accessor methods adjusted. Because the links are intended to be
used only by a specific container class (:code:`Container`) the struct is made protected.

The implementation of the link accessor methods.

.. literalinclude:: ../../unit_tests/ex_IntrusiveDList.cc
   :lines: 64-73

A method to check if the message is in a list.

.. literalinclude:: ../../unit_tests/ex_IntrusiveDList.cc
   :lines: 75-79

The container class for the messages could be implemented as

.. literalinclude:: ../../unit_tests/ex_IntrusiveDList.cc
   :lines: 81-98

The :code:`debug` method takes a format string (:arg:`fmt`) and an arbitrary set of arguments, formats
the arguments in to the string, and adds the new message to the list.

.. literalinclude:: ../../unit_tests/ex_IntrusiveDList.cc
   :lines: 122-131

The :code:`print` method demonstrates the use of the range :code:`for` loop on a list.

.. literalinclude:: ../../unit_tests/ex_IntrusiveDList.cc
   :lines: 142-148

The maximum severity level can also be computed even more easily using :code:`std::max_element`.
This find the element with the maximum severity and returns that severity, or :code:`LVL_DEBUG` if
no element is found (which happens if the list is empty).

.. literalinclude:: ../../unit_tests/ex_IntrusiveDList.cc
   :lines: 134-140

Other methods for the various severity levels would be implemented in a similar fashion. Because the
intrusive list does not do memory management, the container must clean that up itself, as in the
:code:`clear` method. A bit of care must be exercised because the links are in the elements, and
these links are used for iteration therefore using an iterator that references a deleted object is
risky. One approach, illustrated here, is to use :libswoc:`IntrusiveDList::take_head` to remove the
element before destroying it. Another option is to allocation the elements in a :class:`MemArena` to
avoid the need for any explicit cleanup.

.. literalinclude:: ../../unit_tests/ex_IntrusiveDList.cc
   :lines: 106-114

There is a method specifically for this situation as well, :libswoc:`IntrusiveDList::apply` which
applies a functor to every element in the list in such a way that even if elements are deleted or
removed by the functor, iteration is still successful.

In some cases the elements of the list are subclasses and the links are declared in a super class
and are therefore of the super class type. For instance, in the unit test a class :code:`Thing` is
defined for testing.

.. literalinclude:: ../../unit_tests/ex_IntrusiveDList.cc
   :lines: 158

Later on, to validate use on a subclass, :code:`PrivateThing` is defined as a subclass of
:code:`Thing`.

.. literalinclude:: ../../unit_tests/ex_IntrusiveDList.cc
   :lines: 168

However, the link members :code:`_next` and :code:`_prev` are of type :code:`Thing*` but the
descriptor for a list of :code:`PrivateThing` must have link accessors that return
:code:`PrivateThing *&`. To make this easier a conversion template function is provided,
:code:`ts::ptr_ref_cast<X, T>` that converts a member of type :code:`T*` to a reference to a pointer
to :code:`X`, e.g. :code:`X*&`. This is used in the setup for testing :code:`PrivateThing`.

.. literalinclude:: ../../unit_tests/ex_IntrusiveDList.cc
   :lines: 176-187

While this can be done directly with :code:`reinterpret_cast<>`, use of :code:`ts::ptr_cast` avoids
typographic errors and warnings about type punning caused by :code:`-fstrict-aliasing`.

For convenience there are two helper templates provided. The first is :code:`IntrusiveLinkage` which, given the
link members, creates the linkage structure required for :code:`IntrusiveDList`. For the :code:`Thing` class
above, this looks like

.. literalinclude:: ../../unit_tests/ex_IntrusiveDList.cc
   :lines: 158-165

For subclasses, there is :code:`IntrusiveLinkageRebind<T,L>` which rebinds the superclass links to
the subclass. The first template argument is the class to which to rebind (the subclass) and the
second template argument is the linkage structure to rebind. For :code:`PrivateThing2`, which is
identical to :code:`PrivateThing` except for using this helper template, the result is

.. literalinclude:: ../../unit_tests/ex_IntrusiveDList.cc
   :lines: 196-211

The :code:`friend` is required because :code:`Thing` is a non-public superclass and therefore is not
by default accessible from the helper template. In :code:`PrivateThing` the implementation is directly
in the subclass and therefore has access to the superclass.

Design Notes
************

The historic goal of this class is to replace the :code:`DLL` list support in Traffic Server. The
benefits of this are

*  Remove dependency on the C preprocessor.

*  Provide greater flexibility in the internal link members. Because of the use of the descriptor
   and its static methods, the links can be anywhere in the object, including in nested structures
   or super classes. The links are declared like normal members and do not require specific macros.

*  Provide STL compliant iteration. This makes the class easier to use in general and particularly
   in the case of range :code:`for` loops.

*  Track the number of items in the list.

*  Provide queue support, which is of such low marginal expense there is, IMHO, no point in
   providing a separate class for it.



