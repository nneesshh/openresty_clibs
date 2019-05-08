#!python

import os, subprocess, platform, sys, glob


def add_sources(sources, dir, extension, ignores=[]):
    for f in os.listdir(dir):
        if f.endswith('.' + extension):
            ignore = False
            for i in ignores:
                if f.endswith(i):
                    ignore = True
            if not ignore:
                sources.append(dir + '/' + f)


# Try to detect the host platform automatically
# This is used if no `platform` argument is passed
if sys.platform.startswith('linux'):
    host_platform = 'linux'
elif sys.platform == 'darwin':
    host_platform = 'osx'
elif sys.platform == 'win32':
    host_platform = 'windows'
else:
    raise ValueError('Could not detect platform automatically, please specify with platform=<platform>')

opts = Variables([], ARGUMENTS)

opts.Add(EnumVariable('platform', 'Target platform', host_platform, ('linux', 'osx', 'windows')))
opts.Add(EnumVariable('bits', 'Target platform bits', 'default', ('default', '32', '64')))
opts.Add(BoolVariable('use_llvm', 'Use the LLVM compiler - only effective when targeting Linux', False))
opts.Add(BoolVariable('use_mingw', 'Use the MinGW compiler - only effective on Windows', False))

# Must be the same setting as used for cpp_bindings
opts.Add(EnumVariable('target', 'Compilation target', 'release', ('debug', 'release')))
#opts.Add(PathVariable('headers_dir', 'Path to the directory containing header files', 'addons'))


bits = opts.args.get('bits', 'default')
print("bits: " + bits)

if bits == 'default':
    target_arch = 'x86'
elif bits == '64':
    target_arch = 'amd64'
else:
    target_arch ='x86'
env = Environment(TARGET_ARCH=target_arch)
opts.Update(env)
Help(opts.GenerateHelpText(env))

print("================ platform ================")
print(env['platform'])

print("================ TARGET_ARCH ================")
print(env['TARGET_ARCH'])

if env['platform'] == 'linux':
    if env['use_llvm']:
        env['CXX'] = 'clang++'

    env.Append(CCFLAGS=['-fPIC', '-g', '-Wwrite-strings'])
    env.Append(LINKFLAGS=["-Wl,-R,'$$ORIGIN'"])

    if env['target'] == 'debug':
        env.Append(CCFLAGS=['-Og'])
    elif env['target'] == 'release':
        env.Append(CCFLAGS=['-O3'])

    if env['bits'] == '64':
        env.Append(CCFLAGS=['-m64'])
        env.Append(LINKFLAGS=['-m64'])
    elif env['bits'] == '32':
        env.Append(CCFLAGS=['-m32'])
        env.Append(LINKFLAGS=['-m32'])

elif env['platform'] == 'osx':
    if env['bits'] == '32':
        raise ValueError('Only 64-bit builds are supported for the macOS target.')

    env.Append(CCFLAGS=['-g', '-arch', 'x86_64'])
    env.Append(LINKFLAGS=['-arch', 'x86_64', '-framework', 'Cocoa', '-Wl,-undefined,dynamic_lookup'])

    if env['target'] == 'debug':
        env.Append(CCFLAGS=['-Og'])
    elif env['target'] == 'release':
        env.Append(CCFLAGS=['-O3'])

elif env['platform'] == 'windows':
    # This makes sure to keep the session environment variables on Windows
    # This way, you can run SCons in a Visual Studio 2017 prompt and it will find all the required tools
    if env['bits'] == '64' or env['bits'] == '32':
        env.Override(os.environ)

    if host_platform == 'windows' and not env['use_mingw']:
        # MSVC
        env.Append(LINKFLAGS=['/WX'])
        if env['target'] == 'debug':
            env.Append(CCFLAGS=['/EHsc', '/D_DEBUG', '/MDd'])
        elif env['target'] == 'release':
            env.Append(CCFLAGS=['/O2', '/EHsc', '/DNDEBUG', '/MD'])
    else:
        # MinGW
        if env['bits'] == '64':
            env['CXX'] = 'x86_64-w64-mingw32-g++'
        elif env['bits'] == '32':
            env['CXX'] = 'i686-w64-mingw32-g++'

        env.Append(CCFLAGS=['-g', '-O3', '-Wwrite-strings'])
        env.Append(LINKFLAGS=['--static', '-Wl,--no-undefined', '-static-libgcc', '-static-libstdc++'])


print("================ bits ================")
print(env['bits'])

print("================ target ================")
print(env['target'])
       
if env['platform'] == 'linux':
    env.Append(CCFLAGS=['-D _CONSOLE',
                        '-D _CRT_SECURE_NO_WARNINGS',
                        '-D LUA_USE_LINUX',
    ])

elif env['platform'] == 'windows':
    env.Append(CCFLAGS=['/DWIN32',
                        '/D_CONSOLE',
                        '/D_CRT_SECURE_NO_WARNINGS',
                        '/DLUA_BUILD_AS_DLL',
        ])

print("================ CCFLAGS ================")
print(env['CCFLAGS'])

# lualib
lualib_cpp_paths = []
if env['platform'] == 'linux':
    # cpp paths
    lualib_cpp_paths = lualib_cpp_paths + ['depend/luajit/include/luajit-2.1']
    env.Append(CPPPATH=lualib_cpp_paths)

    # libpath
    env.Append(LIBPATH=['depend/luajit/lib'])

elif env['platform'] == 'windows':
    # cpp paths
    lualib_cpp_paths = lualib_cpp_paths + ['depend/lua-5.1.5/src']
    env.Append(CPPPATH=lualib_cpp_paths)

    lualib_sources = []
    add_sources(lualib_sources, 'depend/lua-5.1.5/src', 'c', ['lua.c', 'luac.c'])

    lualib = env.SharedLibrary(
        target = 'depend/lua-5.1.5/lib/lua51',
        source = lualib_sources
    )
    
    # libpath
    env.Append(LIBPATH=['depend/lua-5.1.5/lib'])

print("================ LIBPATH ================")
print(env['LIBPATH'])

lua_platform_libs = []
if env['platform'] == 'linux':
    lua_platform_libs = lua_platform_libs + ['luajit-5.1']
elif env['platform'] == 'windows':
    lua_platform_libs = lua_platform_libs + ['lua51']

print("================ lua_platform_libs ================")
print(lua_platform_libs)

# lua
if env['platform'] == 'windows':
    lua_sources = ['depend/lua-5.1.5/src/lua.c']

    lua = env.Program(
        target = 'depend/lua-5.1.5/bin/lua',
        source = lua_sources,
        LIBS = lua_platform_libs
    )

# common
common_env = env.Clone()

# remove lib prefix for linux
if common_env['platform'] == 'linux':
    common_env['SHLIBPREFIX']=''

if common_env['platform'] == 'linux':
    common_env.Append(CCFLAGS=['-D LUA_LIB'])
elif common_env['platform'] == 'windows':
    common_env.Append(CCFLAGS=['/DLUA_LIB'])

print("================ common_env CCFLAGS ================")
print(common_env['CCFLAGS'])

# lpeg lib
lpeglib_sources = []
add_sources(lpeglib_sources, 'lpeg/src', 'c')

lpeg = common_env.SharedLibrary(
    target = 'clibs/lpeg',
    source = lpeglib_sources,
    LIBS = lua_platform_libs
)

# cjson lib
cjsonlib_sources = [
    'lua-cjson-2.1.0.7rc1/fpconv.c',
    'lua-cjson-2.1.0.7rc1/strbuf.c',
    'lua-cjson-2.1.0.7rc1/lua_cjson.c',
]

cjson = common_env.SharedLibrary(
    target = 'clibs/cjson',
    source = cjsonlib_sources,
    LIBS = lua_platform_libs
)

# mongo lib
mongo_env = common_env.Clone()

if mongo_env['platform'] == 'linux':
    mongo_env.Append(CCFLAGS=[
        '-D BSON_COMPILATION',
        '-D BSON_STATIC',
        '-D MONGOC_COMPILATION',
        '-D MONGOC_STATIC',
    ]
)
elif mongo_env['platform'] == 'windows':
    mongo_env.Append(CCFLAGS=[
        '/D_WINDOWS',
        '/DBSON_COMPILATION',
        '/DBSON_STATIC',
        '/DMONGOC_COMPILATION',
        '/DMONGOC_STATIC',
    ]
)

mongo_cpp_paths = []
if mongo_env['platform'] == 'linux':
    mongo_cpp_paths = mongo_cpp_paths + [
        'mongo-c-driver-1.14.0/src/libbson/src/LINUX-Code',
        'mongo-c-driver-1.14.0/src/libmongoc/src/LINUX-Code',
    ]
elif mongo_env['platform'] == 'windows':
    mongo_cpp_paths = mongo_cpp_paths + [
        'mongo-c-driver-1.14.0/src/libbson/src/WIN32-Code',
        'mongo-c-driver-1.14.0/src/libmongoc/src/WIN32-Code',
    ]

mongo_cpp_paths = mongo_cpp_paths + [
    'mongo-c-driver-1.14.0/src/common',
    'mongo-c-driver-1.14.0/build/src/zlib-1.2.11',
    'mongo-c-driver-1.14.0/src/zlib-1.2.11',
    'mongo-c-driver-1.14.0/src/libbson/src/bson',
    'mongo-c-driver-1.14.0/src/libbson/src',
    'mongo-c-driver-1.14.0/src/libmongoc/src/mongoc',
    'mongo-c-driver-1.14.0/src/libmongoc/src',
]
mongo_env.Append(CPPPATH=mongo_cpp_paths)

mongolib_sources = []
add_sources(mongolib_sources, 'mongo-c-driver-1.14.0/src/common', 'c')
add_sources(mongolib_sources, 'mongo-c-driver-1.14.0/src/libbson/src/bson', 'c')
add_sources(mongolib_sources, 'mongo-c-driver-1.14.0/src/libbson/src/jsonsl', 'c')
add_sources(mongolib_sources, 'mongo-c-driver-1.14.0/src/libmongoc/src/mongoc', 'c')
add_sources(mongolib_sources, 'mongo-c-driver-1.14.0/src/zlib-1.2.11', 'c')
add_sources(mongolib_sources, 'lua-mongo-master/src', 'c')

mongo_platform_libs = []
if mongo_env['platform'] == 'linux':
    mongo_platform_libs = mongo_platform_libs + [
        'rt',
        'resolv',
    ]
elif mongo_env['platform'] == 'windows':
    mongo_platform_libs = mongo_platform_libs + [
        'secur32.lib',
        'crypt32.lib',
        'Shlwapi.lib',
        'Bcrypt.lib',
        'Dnsapi.lib',
        'ws2_32.lib',
        'Advapi32.lib',
    ]

mongo = mongo_env.SharedLibrary(
    target = 'clibs/mongo',
    source = mongolib_sources,
    LIBS = lua_platform_libs + mongo_platform_libs
)

# openssl lib
openssl_env = common_env.Clone()

openssllib_sources = [
    'luaossl-rel-20180530/src/openssl.c',
]

openssl_cpp_paths = []
if openssl_env['platform'] == 'linux':
    openssl_cpp_paths = openssl_cpp_paths + ['depend/openssl/include']
elif openssl_env['platform'] == 'windows':
    openssl_cpp_paths = openssl_cpp_paths + ['depend/pgsql/include']
openssl_env.Append(CPPPATH=openssl_cpp_paths)

if openssl_env['platform'] == 'linux':
    openssl_env.Append(LIBPATH=['depend/openssl/lib'])
elif openssl_env['platform'] == 'windows':
    openssl_env.Append(LIBPATH=['depend/pgsql/lib'])

openssl_platform_libs = []
if openssl_env['platform'] == 'linux':
    openssl_platform_libs = openssl_platform_libs + [
        'ssl',
        'crypto',
    ]
elif openssl_env['platform'] == 'windows':
    openssl_platform_libs = openssl_platform_libs + [
        'libeay32.lib',
        'ssleay32.lib',
        'ws2_32.lib',
        'Advapi32.lib',
    ]

openssl = openssl_env.SharedLibrary(
    target = 'clibs/_openssl',
    source = openssllib_sources,
    LIBS = lua_platform_libs + openssl_platform_libs
)

# mime lib
mime_env = common_env.Clone()

if mime_env['platform'] == 'windows':
    mime_env.Append(CCFLAGS=[
        '/D_WINDOWS',
        '/DMIME_API=__declspec(dllexport)',
    ]
)

mime_sources = [
    'luasocket-master/src/mime.c',
]
mime_compat_object = mime_env.SharedObject('luasocket-master/src/mime-compat', 'luasocket-master/src/compat.c')

mime = mime_env.SharedLibrary(
    target = 'clibs/mime/core',
    source = mime_compat_object + mime_sources,
    LIBS = lua_platform_libs
)

# socket lib
socket_env = common_env.Clone()

if socket_env['platform'] == 'windows':
    socket_env.Append(CCFLAGS=[
        '/D_WINDOWS',
        '/DLUASOCKET_API=__declspec(dllexport)',
    ]
)

socket_sources = [
    'luasocket-master/src/auxiliar.c',
    'luasocket-master/src/buffer.c',
    'luasocket-master/src/except.c',
    'luasocket-master/src/inet.c',
    'luasocket-master/src/io.c',
    'luasocket-master/src/luasocket.c',
    'luasocket-master/src/options.c',
    'luasocket-master/src/select.c',
    'luasocket-master/src/tcp.c',
    'luasocket-master/src/timeout.c',
    'luasocket-master/src/udp.c',
]
socket_compat_object = socket_env.SharedObject('luasocket-master/src/socket-compat', 'luasocket-master/src/compat.c')

if socket_env['platform'] == 'linux':
    socket_sources = socket_sources + ['luasocket-master/src/usocket.c']
elif socket_env['platform'] == 'windows':
    socket_sources = socket_sources + ['luasocket-master/src/wsocket.c']

socket_platform_libs = []
if socket_env['platform'] == 'windows':
    socket_platform_libs = socket_platform_libs + [
        'ws2_32.lib',
    ]

socket = socket_env.SharedLibrary(
    target = 'clibs/socket/core',
    source = socket_compat_object + socket_sources,
    LIBS = lua_platform_libs + socket_platform_libs
)

# lua c utility lib
luc_env = common_env.Clone()

if luc_env['platform'] == 'windows':
    luc_env.Append(CCFLAGS=[
        '/D_WINDOWS',
    ]
)

lcu_sources = [
    'lua-c-utility/src/lcu.c',
    'lua-c-utility/src/lcu_netinfo.c',
    'lua-c-utility/src/lcu_platform.c',
    'lua-c-utility/src/memory_stream.c',
]

lcu = luc_env.SharedLibrary(
    target = 'clibs/lcu',
    source = lcu_sources,
    LIBS = lua_platform_libs
)

# lua protobuf lib
lpb_env = common_env.Clone()

if lpb_env['platform'] == 'windows':
    lpb_env.Append(CCFLAGS=[
        '/D_WINDOWS',
    ]
)

lpb_sources = [
    'lua-protobuf-master/src/lpb.c',
]

lpb = lpb_env.SharedLibrary(
    target = 'clibs/lpb',
    source = lpb_sources,
    LIBS = lua_platform_libs
)

if env['platform'] == 'linux':
    Default(lpeg, cjson, mongo, openssl, mime, socket, lcu, lpb)
elif env['platform'] == 'windows':
    Default(lualib, lua, lpeg, cjson, mongo, openssl, mime, socket, lcu, lpb)