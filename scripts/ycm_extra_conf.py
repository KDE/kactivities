import os
import ycm_core
from clang_helpers import PrepareClangFlags

# Set this to the absolute path to the folder (NOT the file!) containing the
# compile_commands.json file to use that instead of 'flags'. See here for
# more details: http://clang.llvm.org/docs/JSONCompilationDatabase.html
# Most projects will NOT need to set this to anything; you can just change the
# 'flags' list of compilation flags. Notice that YCM itself uses that approach.
compilation_database_folder = ''

# These are the compilation flags that will be used in case there's no
# compilation database set.
flags = [
'-Wall',
'-Wextra',
'-Werror',
'-Wc++98-compat',
'-Wno-long-long',
'-Wno-variadic-macros',
'-DUSE_CLANG_COMPLETER',
'-std=c++11',
'-x',
'c++',
'-I/opt/kde/build/core/libs/kactivities/src',
'-I/opt/kde/src/core/libs/kactivities/src',
'-I/opt/kde/usr/kde/include',
'-I/opt/kde/usr/kde/include/KDE',
'-I/usr/include',
'-I/usr/include/qt4',
'-I/usr/include/qt4/Qt',
'-I/usr/include/qt4/Qt3Support',
'-I/usr/include/qt4/QtCore',
'-I/usr/include/qt4/QtDBus',
'-I/usr/include/qt4/QtDeclarative',
'-I/usr/include/qt4/QtDesigner',
'-I/usr/include/qt4/QtDesigner',
'-I/usr/include/qt4/QtGui',
'-I/usr/include/qt4/QtHelp',
'-I/usr/include/qt4/QtNetwork',
'-I/usr/include/qt4/QtOpenGL',
'-I/usr/include/qt4/QtScript',
'-I/usr/include/qt4/QtScriptTools',
'-I/usr/include/qt4/QtSql',
'-I/usr/include/qt4/QtSvg',
'-I/usr/include/qt4/QtTest',
'-I/usr/include/qt4/QtUiTools',
'-I/usr/include/qt4/QtWebKit',
'-I/usr/include/qt4/QtXml',
'-I/usr/include/qt4/QtXmlPatterns',
'-I/usr/share/qt4/mkspecs/default',
'-D_BSD_SOURCE',
'-DQT_NO_DEBUG_OUTPUT',
'-DQT_USE_FAST_CONCATENATION',
'-DQT_USE_FAST_OPERATOR_PLUS',
'-D_XOPEN_SOURCE=500',
'-D_BSD_SOURCE',
'-DQT_NO_STL',
'-DQT_NO_CAST_TO_ASCII',
'-D_REENTRANT',
'-DKDE_DEPRECATED_WARNINGS',
'-DKDE4_CMAKE_TOPLEVEL_DIR_LENGTH=22'
]

if compilation_database_folder:
  database = ycm_core.CompilationDatabase( compilation_database_folder )
else:
  database = None


def DirectoryOfThisScript():
  return os.path.dirname( os.path.abspath( __file__ ) )


def MakeRelativePathsInFlagsAbsolute( flags, working_directory ):
  if not working_directory:
    return flags
  new_flags = []
  make_next_absolute = False
  path_flags = [ '-isystem', '-I', '-iquote', '--sysroot=' ]
  for flag in flags:
    new_flag = flag

    if make_next_absolute:
      make_next_absolute = False
      if not flag.startswith( '/' ):
        new_flag = os.path.join( working_directory, flag )

    for path_flag in path_flags:
      if flag == path_flag:
        make_next_absolute = True
        break

      if flag.startswith( path_flag ):
        path = flag[ len( path_flag ): ]
        new_flag = path_flag + os.path.join( working_directory, path )
        break

    if new_flag:
      new_flags.append( new_flag )
  return new_flags


def FlagsForFile( filename ):
  if database:
    # Bear in mind that compilation_info.compiler_flags_ does NOT return a
    # python list, but a "list-like" StringVec object
    compilation_info = database.GetCompilationInfoForFile( filename )
    final_flags = PrepareClangFlags(
        MakeRelativePathsInFlagsAbsolute(
            compilation_info.compiler_flags_,
            compilation_info.compiler_working_dir_ ),
        filename )

    # NOTE: This is just for YouCompleteMe; it's highly likely that your project
    # does NOT need to remove the stdlib flag. DO NOT USE THIS IN YOUR
    # ycm_extra_conf IF YOU'RE NOT 100% YOU NEED IT.
    try:
      final_flags.remove( '-stdlib=libc++' )
    except ValueError:
      pass
  else:
    relative_to = DirectoryOfThisScript()
    final_flags = MakeRelativePathsInFlagsAbsolute( flags, relative_to )

  return {
    'flags': final_flags,
    'do_cache': True
  }
