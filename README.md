jdock
=====

jdock is an extended variant of idock which was originally developed by [@HongjianLi](https://github.com/HongjianLi) and is distributed under the same license.

>idock is a standalone tool for structure-based [virtual screening] powered by fast and flexible ligand docking. It was inspired by [AutoDock Vina], and is hosted on GitHub at https://GitHub.com/HongjianLi/idock under [Apache License 2.0]. idock is also available as a web server at [istar].

jdock keeps full compatibility with idock and the idock branding remains unchanged across the source code. The only naming change is the name of output binary and GitHub repository.

Features
--------

jdock inherits all features of idock in branch v2.x and tracks changes in that branch. Along with some bug fixes, jdock also adds its own functionalities including but not limited to:
* non-standard residue elimination,
* protonation with pKa values,
* per residue energy summarization and emission,
* alternate location indicator choosing,
* non-compulsory RF-score calculation,
* precision mode to avoid the use of grid maps,
* scoring and docking in a single run,
* compatibility with all kinds of line feedings.


Supported operating systems and compilers
-----------------------------------------

* Linux x86_64 and g++ 8.3.1
* Mac OS X x86_64 and clang 8.0.0
* Windows x86_64 and Visual Studio 2019


Compilation from source code
----------------------------

### Compilation of Boost

jdock depends on the `Program Options` and `Asio` components in [Boost C++ Libraries]. Boost 1.70.0 was tested. To compile jdock, first download [Boost 1.70] and unpack the archive to `boost_1_70_0/include`.

Build on Linux run:
```
cd boost_1_70_0/include
./bootstrap.sh
./b2 --build-dir=../build/linux_x64 --stagedir=../ -j 8 link=static address-model=64
```

Or, on Windows run:
```
cd boost_1_70_0\include
bootstrap.bat
b2 --build-dir=../build/win_x64 --stagedir=../ -j 8 link=static address-model=64
```

Then add the path of the `boost_1_70_0` directory the to the BOOST_ROOT environment variable.

### Compilation on Linux

The Makefile uses g++ as the default compiler. To compile, simply run
```
make
```

One may modify the Makefile to use a different compiler or different compilation options.

The generated objects will be placed in the `obj` folder, and the generated executable will be placed in the `../bin` folder.

Optionally, one may copy the output binary to `/usr/bin` by running
```
make setup
```

### Compilation on Windows

Visual Studio 2019 solution and project files are provided. To compile, simply run
```
msbuild /t:Build /p:Configuration=Release
```

Or one may open `idock.sln` in Visual Studio 2019 and do a full rebuild.

The generated objects will be placed in the `obj` folder, and the generated executable will be placed in the `..\bin` folder.


Usage
-----

First add jdock to the PATH environment variable.

To display a full list of available options, simply run the program without arguments
```
jdock
```

The `examples` folder contains several use cases. For example, to dock the ligand TMC278 against HIV-1 RT of PDB ID 2ZD1,
```
jdock -r receptors/2ZD1.pdbqt -l ligands/T27 -x 49.712 -y -28.923 -z 36.824 --size_x 18 --size_y 18 --size_z 20
```

Or one can instruct jdock to load the options from a configuration file
```
cd examples/2ZD1/T27
jdock --config idock.conf
```


Change Log
----------

### 2.2.3a (2019-06-14)

* Initial public release based on idock v2.2.3


Reference
---------

### jdock
To be added.

### idock
Hongjian Li, Kwong-Sak Leung, and Man-Hon Wong. idock: A Multithreaded Virtual Screening Tool for Flexible Ligand Docking. 2012 IEEE Symposium on Computational Intelligence in Bioinformatics and Computational Biology (CIBCB), pp.77-84, San Diego, United States, 9-12 May 2012. [DOI: 10.1109/CIBCB.2012.6217214]


Author
--------------

### jdock
[Maozi Chen]

### idock
[Jacky Lee]


[virtual screening]: http://en.wikipedia.org/wiki/Virtual_screening
[docking]: http://en.wikipedia.org/wiki/Docking_(molecular)
[AutoDock Vina]: http://vina.scripps.edu
[Apache License 2.0]: http://www.apache.org/licenses/LICENSE-2.0
[istar]: http://istar.cse.cuhk.edu.hk/idock
[Boost C++ Libraries]: http://www.boost.org
[doxygen]: http://www.doxygen.org
[semantic versioning]: http://semver.org
[CodePlex]: http://idock.codeplex.com
[DOI: 10.1109/CIBCB.2012.6217214]: http://dx.doi.org/10.1109/CIBCB.2012.6217214
[Jacky Lee]: http://www.cse.cuhk.edu.hk/~hjli
[Maozi Chen]: https://www.linkedin.com/in/maozichen/
[Boost 1.70]: https://www.boost.org/users/history/version_1_70_0.html
