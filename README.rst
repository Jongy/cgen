cgen
====

Pythonic generators in C, for Linux x86-64.

Usage
-----

Build the library::

    make

Then include ``cgen.h`` in your code.

Now, defining a generator is as simple as::

    void mygen(int arg1, int arg2);

Return value should be ``void`` (it's ignored anyway) and you can accept up to ``CGEN_MAX_ARGS`` args
(currently 10).

The generator can use ``yield`` to yield values. ``yield`` accepts a value to yield to the caller, and
returns the value sent by the caller (``0`` if nothing was sent).

The callers instantiates a generator by calling ``generator`` with the generator function and initial
arguments, for example ``generator(mygen, 10, 20)``. It returns a pointer to ``struct gen`` which can
be used by future calls to ``send``/``next``.

``send`` writes the yielded value to a given pointer, and it also accepts a value to send to the
generator, as well as returning ``true`` if the generator can be called again (``false`` when it's
exhausted, on which case it's automatically freed).
``next`` is a shorthand for ``send`` with the value ``0``.

There's also ``yield_from``, you should look in the examples below for usage instructions.

Examples
--------

All examples can be built with ``make examples``.

Simplest:

.. literalinclude:: examples/simple.c

You can send values to the generator (like Python's ``send``):

.. literalinclude:: examples/send.c

And there's ``yield_from`` which behaves like Python's ``yield from`` - another generator is "attached"
to your current consumer, and the generator calling ``yield_from`` resumes execution when that enerator
is exhausted.

.. literalinclude:: examples/yield_from.c

``yield_from`` s can be chained.

See ``test.c`` for more examples.
