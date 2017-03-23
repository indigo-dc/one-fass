import os
import sys
import shutil
#sys.path.append("./share/scons")
sys.path.append("/usr/bin")
#from lex_bison import *

# This is the absolute path where the project is located
cwd=os.getcwd()

# Environment that will be applied to each scons child
main_env=Environment()
main_env['ENV']['PATH']=os.environ['PATH']

# snippet borrowed from http://dev.gentoo.org/~vapier/scons-blows.txt
# makes scons aware of build related environment variables
if os.environ.has_key('CC'):
    main_env['CC'] = os.environ['CC']
if os.environ.has_key('CFLAGS'):
    main_env['CCFLAGS'] += SCons.Util.CLVar(os.environ['CFLAGS'])
if os.environ.has_key('CXX'):
    main_env['CXX'] = os.environ['CXX']
if os.environ.has_key('CXXFLAGS'):
    main_env['CXXFLAGS'] += SCons.Util.CLVar(os.environ['CXXFLAGS'])
if os.environ.has_key('LDFLAGS'):
    main_env['LINKFLAGS'] += SCons.Util.CLVar(os.environ['LDFLAGS'])
else:
    os.environ['LDFLAGS']=""

# Add builders for flex and bison
#add_lex(main_env)
#add_bison(main_env)

# Include dirs
main_env.Append(CPPPATH=[
    cwd+'/include',
])

# Library dirs
main_env.Append(LIBPATH=[
    cwd+'/src/fass',
    cwd+'/src/config',
    cwd+'/src/logger',
    cwd+'/src/rpcm', 
    cwd+'/src/client',
    cwd+'/src/database',
    cwd+'/src/pm'
])

# Compile flags
main_env.Append(CPPFLAGS=[
    "-g",
    "-Wall"
])

# Linking flags & common libraries
main_env.Append(LINKFLAGS=['-g', '-pthread'])
#main_env.Append(LIBS=['z','boost_program_options','jsoncpp'])
main_env.Append(LIBS=['z','boost_program_options','boost_system'])

# jsoncpp parser
main_env.Append(LIBPATH=["/usr/lib/jsoncpp", "/user/lib64/jsoncpp"])
main_env.Append(CPPPATH=["/usr/include/jsoncpp"])

# poco libraries
#main_env.Append(LIBPATH=["/usr/lib64/Poco"])
#main_env.Append(CPPPATH=["/usr/include/Poco"])

#######################
# EXTRA CONFIGURATION #
#######################

# SQLITE
#sqlite_dir=ARGUMENTS.get('sqlite_dir', 'none')
#if sqlite_dir!='none':
#    main_env.Append(LIBPATH=[sqlite_dir+"/lib", sqlite_dir+"/lib64"])
#    main_env.Append(CPPPATH=[sqlite_dir+"/include"])

#sqlite=ARGUMENTS.get('sqlite', 'yes')
#if sqlite=='yes':
#    main_env.Append(sqlite='yes')
#    main_env.Append(CPPFLAGS=["-DSQLITE_DB"])
#    main_env.Append(LIBS=['sqlite3'])
#else:
#    main_env.Append(sqlite='no')

# MySQL
#mysql=ARGUMENTS.get('mysql', 'no')
#if mysql=='yes':
#    main_env.Append(mysql='yes')
#    main_env.Append(CPPFLAGS=["-DMYSQL_DB"])
#    main_env.Append(LIBS=['mysqlclient'])
#else:
#    main_env.Append(mysql='no')

# Flag to compile with xmlrpc-c versions prior to 1.31 (September 2012)
#new_xmlrpc=ARGUMENTS.get('new_xmlrpc', 'no')
#if new_xmlrpc=='yes':
#    main_env.Append(new_xmlrpc='yes')
#else:
#    main_env.Append(new_xmlrpc='no')
#    main_env.Append(CPPFLAGS=["-DOLD_XMLRPC"])

# xmlrpc
xmlrpc_dir=ARGUMENTS.get('xmlrpc', 'none')
if xmlrpc_dir!='none':
    main_env.Append(LIBPATH=[xmlrpc_dir+"/lib", xmlrpc_dir+"/lib64"])
    main_env.Append(CPPPATH=[xmlrpc_dir+"/include"])

# build lex/bison
#build_parsers=ARGUMENTS.get('parsers', 'no')
#if build_parsers=='yes':
#    main_env.Append(parsers='yes')
#else:
#    main_env.Append(parsers='no')

# Rubygem generation
#main_env.Append(rubygems=ARGUMENTS.get('rubygems', 'no'))

# Sunstone minified files generation
#main_env.Append(sunstone=ARGUMENTS.get('sunstone', 'no'))

if not main_env.GetOption('clean'):
#    try:
#        if mysql=='yes':
#            main_env.ParseConfig('mysql_config --cflags --libs')
#    except Exception, e:
#        print ""
#        print "mysql_config was not found in the path"
#        print ""
#        print "Check that mysql development package is installed and"
#        print "mysql_config is in the path. If your mysql config tool"
#        print "is called mysql5_config make a symlink as mysql_config"
#        print "to a directory in the path."
#        print ""
#        exit(-1)


    try:
        main_env.ParseConfig(("LDFLAGS='%s' xmlrpc-c-config"+
            " c++2 abyss-server --libs --cflags") % (os.environ['LDFLAGS'],))
        main_env.ParseConfig(("LDFLAGS='%s' xmlrpc-c-config"+
            " c++2 client --libs --cflags") % (os.environ['LDFLAGS'],))

    except Exception, e:
        print e
        exit(-1)
#else:
#    main_env.Replace(mysql='yes')
#    shutil.rmtree('.xmlrpc_test', True)
#    shutil.rmtree('src/nebula/.xmlrpc_test', True)
#    shutil.rmtree('src/scheduler/.xmlrpc_test', True)


# libxml2
main_env.ParseConfig('xml2-config --libs --cflags')


# SCONS scripts to build
build_scripts=[
    'src/fass/SConstruct',
    'src/config/SConstruct',
    'src/logger/SConstruct',
    'src/rpcm/SConstruct',
    'src/client/SConstruct',
    'src/database/SConstruct',
    'src/pm/SConstruct'
]

for script in build_scripts:
    env=main_env.Clone()
    SConscript(script, exports='env')
