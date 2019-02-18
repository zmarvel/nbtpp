
# libnbtpp

C++ library for reading (and eventually writing) NBT files.

# Building

```shell
$ autoreconf -i
$ ./configure
$ make
```

# Running tests

Build and run tests with
```shell
$ make check
```

Look at `test_nbt.log` for test results of individual cases, or you can run
`./test_nbt` by hand for color-coded output.

# License

Copyright (C) 2019  Zack Marvel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
