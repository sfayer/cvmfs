#
# Define the sources needed to compile the different
# executables in this directory
#

set (CVMFS_LOADER_SOURCES
  smalloc.h atomic.h shortstring.h fs_traversal.h
  platform.h platform_linux.h platform_osx.h
  logging_internal.h logging.h logging.cc
  hash.h hash.cc
  util.h util.cc
  options.h options.cc
  globals.h globals.cc
  loader_talk.h loader_talk.cc
  loader.h loader.cc
)

set (CVMFS_CLIENT_SOURCES
  smalloc.h
  logging.cc logging.h logging_internal.h
  tracer.h tracer.cc atomic.h
  duplex_sqlite3.h duplex_curl.h
  signature.h signature.cc
  quota.h quota.cc
  hash.h hash.cc
  cache.h cache.cc
  platform.h platform_osx.h platform_linux.h
  monitor.h monitor.cc
  util.cc util.h
  duplex_zlib.h compression.h compression.cc
  download.cc download.h
  manifest.h manifest.cc
  manifest_fetch.h manifest_fetch.cc
  murmur.h smallhash.h
  bigvector.h
  lru.h
  globals.h globals.cc
  peers.h peers.cc
  sql.h sql.cc
  catalog_sql.h catalog_sql.cc
  catalog.h catalog.cc
  catalog_mgr.h catalog_mgr.cc
  catalog_counters.h catalog_counters_impl.h catalog_counters.cc
  directory_entry.h directory_entry.cc
  shortstring.h
  file_chunk.h file_chunk.cc
  fs_traversal.h
)


set (CVMFS2_DEBUG_SOURCES
  ${CVMFS_CLIENT_SOURCES}

  options.cc options.h
  talk.h talk.cc
  nfs_maps.h nfs_maps.cc
  nfs_shared_maps.h nfs_shared_maps.cc
  glue_buffer.h glue_buffer.cc
  loader.h compat.cc compat.h
  history.h history.cc
  cvmfs.h cvmfs.cc
)

set (CVMFS2_SOURCES
  ${CVMFS2_DEBUG_SOURCES}
)

set (LIBCVMFS_SOURCES
  ${CVMFS_CLIENT_SOURCES}
  libcvmfs_int.h libcvmfs_int.cc
  libcvmfs.h libcvmfs.cc
)

set (CVMFS_FSCK_SOURCES
  platform.h platform_linux.h platform_osx.h
  logging_internal.h logging.h logging.cc
  smalloc.h
  atomic.h
  duplex_zlib.h compression.cc compression.h
  hash.cc hash.h
  util.cc util.h
  cvmfs_fsck.cc)

set (CVMFS_SWISSKNIFE_SOURCES
  smalloc.h atomic.h
  platform.h platform_linux.h platform_osx.h
  duplex_zlib.h duplex_sqlite3.h
  logging.cc logging.h logging_internal.h
  globals.cc globals.h
  manifest.h manifest.cc
  manifest_fetch.h manifest_fetch.cc
  hash.h hash.cc
  compression.h compression.cc
  util.h util.cc
  util_concurrency.h util_concurrency_impl.h util_concurrency.cc

  upload_file_chunker.h upload_file_chunker.cc
  upload_file_chunker_xor32.h upload_file_chunker_xor32.cc
  upload_file_processor.h upload_file_processor.cc
  upload.h upload.cc
  upload_spooler_definition.h upload_spooler_definition.cc
  upload_facility.h upload_facility.cc
  upload_local.h upload_local.cc
  upload_riak.h upload_riak.cc
  upload_spooler_result.h
  json_document.h json_document.cc

  duplex_curl.h download.h download.cc
  signature.cc signature.h

  fs_traversal.h
  sync_item.h sync_item.cc
  sync_union.h sync_union.cc
  sync_mediator.h sync_mediator.cc

  file_chunk.h file_chunk.cc
  directory_entry.h directory_entry.cc
  shortstring.h
  catalog_traversal.h
  sql.h sql.cc
  catalog_sql.h catalog_sql.cc
  catalog.h catalog.cc
  catalog_rw.h catalog_rw.cc
  catalog_mgr.h catalog_mgr.cc
  catalog_mgr_rw.h catalog_mgr_rw.cc
  catalog_counters.h catalog_counters_impl.h catalog_counters.cc
  history.h history.cc

  swissknife_zpipe.h swissknife_zpipe.cc
  swissknife_check.h swissknife_check.cc
  swissknife_lsrepo.h swissknife_lsrepo.cc
  swissknife_pull.h swissknife_pull.cc
  swissknife_sign.h swissknife_sign.cc
  swissknife_sync.h swissknife_sync.cc
  swissknife_info.h swissknife_info.cc
  swissknife_history.h swissknife_history.cc
  swissknife_migrate.h swissknife_migrate.cc
  swissknife.h swissknife.cc)


set (CVMFS_SWISSKNIFE_DEBUG_SOURCES
  ${CVMFS_SWISSKNIFE_SOURCES}
)

#
# configure some compiler flags for proper build
#

set (CVMFS_LOADER_CFLAGS "${CVMFS_LOADER_CFLAGS} -DCVMFS_CLIENT -DCVMFS_NAMESPACE_GUARD=loader")

set (CVMFS2_CFLAGS "${CVMFS2_CFLAGS} -DCVMFS_CLIENT -D_FILE_OFFSET_BITS=64 -fexceptions")
set (CVMFS2_DEBUG_CFLAGS "${CVMFS2_DEBUG_CFLAGS} ${CVMFS2_CFLAGS} -D_FILE_OFFSET_BITS=64 -O0 -DDEBUGMSG")

set (CVMFS_SWISSKNIFE_DEBUG_CFLAGS "${CVMFS_SWISSKNIFE_DEBUG_CFLAGS} -DDEBUGMSG -g")

set (CVMFS_FSCK_CFLAGS "${CVMFS_FSCK_CFLAGS} -DCVMFS_CLIENT")

#
# set some include and linking stuff
#
set (CVMFS2_LD_FLAGS "${CVMFS2_LD_FLAGS} -ldl -lm")
include_directories (${INCLUDE_DIRECTORIES})

#
# do the job!
#
# for every target you can define a <targetname>_CFLAGS and <targetname>_LD_FLAGS
# variable to pass some target specific build options.
#

if (BUILD_CVMFS)

  add_executable (cvmfs2 ${CVMFS_LOADER_SOURCES})
  add_library (cvmfs_fuse_debug SHARED  ${CVMFS2_DEBUG_SOURCES})
  add_library (cvmfs_fuse SHARED ${CVMFS2_SOURCES})
  add_executable (cvmfs_fsck ${CVMFS_FSCK_SOURCES})

  if (LIBFUSE_BUILTIN)
    add_dependencies (cvmfs_fuse_debug libfuse) # here it does not matter if libfuse or libfuse4x
    add_dependencies (cvmfs_fuse libfuse)       # at this point only the appropriate one is defined
  endif (LIBFUSE_BUILTIN)

  if (LIBCURL_BUILTIN)
    add_dependencies (cvmfs_fuse_debug libcares libcurl)
    add_dependencies (cvmfs_fuse libcares libcurl)
  endif (LIBCURL_BUILTIN)

  if (SQLITE3_BUILTIN)
    add_dependencies (cvmfs_fuse_debug sqlite3)
    add_dependencies (cvmfs_fuse sqlite3)
  endif (SQLITE3_BUILTIN)

  if (ZLIB_BUILTIN)
    add_dependencies (cvmfs_fuse_debug zlib)
    add_dependencies (cvmfs_fuse zlib)
    add_dependencies (cvmfs_fsck zlib)
  endif (ZLIB_BUILTIN)

  if (SPARSEHASH_BUILTIN)
    add_dependencies (cvmfs_fuse_debug sparsehash)
    add_dependencies (cvmfs_fuse sparsehash)
  endif (SPARSEHASH_BUILTIN)

  if (LEVELDB_BUILTIN)
    add_dependencies (cvmfs_fuse_debug libleveldb)
    add_dependencies (cvmfs_fuse libleveldb)
  endif (LEVELDB_BUILTIN)

  # set some properties
  set (CVMFS_LOADER_LD_FLAGS "${CVMFS_LOADER_LD_FLAGS} -ldl -lm")
  set_target_properties (cvmfs2 PROPERTIES COMPILE_FLAGS "${CVMFS_LOADER_CFLAGS}" LINK_FLAGS "${CVMFS_LOADER_LD_FLAGS}")
  set_target_properties (cvmfs_fuse_debug PROPERTIES COMPILE_FLAGS "${CVMFS2_DEBUG_CFLAGS}" LINK_FLAGS "${CVMFS2_DEBUG_LD_FLAGS}")
  set_target_properties (cvmfs_fuse PROPERTIES COMPILE_FLAGS "${CVMFS2_CFLAGS}" LINK_FLAGS "${CVMFS2_LD_FLAGS}")
  set_target_properties (cvmfs_fsck PROPERTIES COMPILE_FLAGS "${CVMFS_FSCK_CFLAGS}" LINK_FLAGS "${CVMFS_FSCK_LD_FLAGS}")

  set_target_properties (cvmfs_fuse PROPERTIES VERSION ${CernVM-FS_VERSION_STRING})
  set_target_properties (cvmfs_fuse_debug PROPERTIES VERSION ${CernVM-FS_VERSION_STRING})

  # link the stuff (*_LIBRARIES are dynamic link libraries *_archive are static link libraries ... one of them will be empty for each dependency)
  target_link_libraries (cvmfs2 ${CVMFS_LOADER_LIBS} ${OPENSSL_LIBRARIES} fuse ${RT_LIBRARY} pthread dl)
  target_link_libraries (cvmfs_fuse_debug    ${CVMFS2_DEBUG_LIBS} ${SQLITE3_LIBRARY} ${CURL_LIBRARIES} ${ZLIB_LIBRARIES} ${LEVELDB_LIBRARIES} ${OPENSSL_LIBRARIES} ${FUSE_LIBRARIES} ${LIBFUSE_ARCHIVE} ${SQLITE3_ARCHIVE} ${LIBCURL_ARCHIVE} ${LEVELDB_ARCHIVE} ${CARES_ARCHIVE} ${ZLIB_ARCHIVE} ${RT_LIBRARY} pthread dl)
  target_link_libraries (cvmfs_fuse      ${CVMFS2_LIBS} ${SQLITE3_LIBRARY} ${CURL_LIBRARIES} ${ZLIB_LIBRARIES} ${LEVELDB_LIBRARIES} ${OPENSSL_LIBRARIES} ${FUSE_LIBRARIES} ${LIBFUSE_ARCHIVE} ${SQLITE3_ARCHIVE} ${LIBCURL_ARCHIVE} ${LEVELDB_ARCHIVE} ${CARES_ARCHIVE} ${ZLIB_ARCHIVE} ${RT_LIBRARY} pthread dl)
  target_link_libraries (cvmfs_fsck    ${CVMFS_FSCK_LIBS} ${ZLIB_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_ARCHIVE} pthread)

endif (BUILD_CVMFS)

if (BUILD_LIBCVMFS)
  # libcvmfs_only.a is a static lib of cvmfs without externals
  # libcvmfs.a includes the externals as well
  add_library (cvmfs_only STATIC ${LIBCVMFS_SOURCES})
  set (LIBCVMFS_LIBS ${LIBCVMFS_LIBS} ${CMAKE_CURRENT_BINARY_DIR}/libcvmfs_only.a)

  if (LIBCURL_BUILTIN)
    add_dependencies (cvmfs_only libcares libcurl)
    set (LIBCVMFS_LIBS ${CARES_ARCHIVE} ${LIBCVMFS_LIBS} )
    set (LIBCVMFS_LIBS ${LIBCURL_ARCHIVE} ${LIBCVMFS_LIBS} )
  endif (LIBCURL_BUILTIN)

  if (SQLITE3_BUILTIN)
    add_dependencies (cvmfs_only sqlite3)
    set (LIBCVMFS_LIBS ${SQLITE3_ARCHIVE}  ${LIBCVMFS_LIBS} )
  endif (SQLITE3_BUILTIN)

  if (ZLIB_BUILTIN)
    add_dependencies (cvmfs_only zlib)
    set (LIBCVMFS_LIBS ${ZLIB_ARCHIVE} ${LIBCVMFS_LIBS} )
  endif (ZLIB_BUILTIN)

  if (SPARSEHASH_BUILTIN)
    add_dependencies (cvmfs_only sparsehash)
  endif (SPARSEHASH_BUILTIN)

  set_target_properties (cvmfs_only PROPERTIES COMPILE_FLAGS "${CVMFS2_CFLAGS}")

  if (NOT MACOSX)
    # We do not (yet) have a recipe for localizing private symbols
    # in Mac OS X, so skip that step on Mac
    set (LIBCVMFS_PUBLIC_SYMS "-public" "${CMAKE_CURRENT_SOURCE_DIR}/libcvmfs_public_syms.txt")
  endif (NOT MACOSX)

  add_custom_command(
    OUTPUT libcvmfs.a
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/combine_libs ${LIBCVMFS_PUBLIC_SYMS} libcvmfs.a ${LIBCVMFS_LIBS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${LIBCVMFS_LIBS}
  )

  # dummy target to cause merged libcvmfs to be produced
  add_custom_target (libcvmfs ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libcvmfs.a)
  add_dependencies (libcvmfs cvmfs_only)

endif (BUILD_LIBCVMFS)

if (BUILD_SERVER)
  # create targets with according sources and static link libraries
  add_executable (cvmfs_swissknife ${CVMFS_SWISSKNIFE_SOURCES})

  include (${VJSON_BUILTIN_LOCATION}/CVMFS-CMakeLists.txt)
  add_dependencies (cvmfs_swissknife libvjson)
  set (INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${VJSON_BUILTIN_LOCATION}/src)
  include_directories (${INCLUDE_DIRECTORIES})

  if (LIBCURL_BUILTIN)
    add_dependencies (cvmfs_swissknife libcares libcurl)
  endif (LIBCURL_BUILTIN)

  if (SQLITE3_BUILTIN)
    add_dependencies (cvmfs_swissknife sqlite3)
  endif (SQLITE3_BUILTIN)

  if (ZLIB_BUILTIN)
    add_dependencies (cvmfs_swissknife zlib)
  endif (ZLIB_BUILTIN)

  set_target_properties (cvmfs_swissknife PROPERTIES COMPILE_FLAGS "${CVMFS_SWISSKNIFE_CFLAGS}" LINK_FLAGS "${CVMFS_SWISSKNIFE_LD_FLAGS}")

  # link the stuff (*_LIBRARIES are dynamic link libraries)
  target_link_libraries (cvmfs_swissknife  ${CVMFS_SWISSKNIFE_LIBS} ${SQLITE3_LIBRARY} ${CURL_LIBRARIES} ${ZLIB_LIBRARIES} ${OPENSSL_LIBRARIES} ${LIBCURL_ARCHIVE} ${CARES_ARCHIVE} ${SQLITE3_ARCHIVE} ${ZLIB_ARCHIVE} ${RT_LIBRARY} ${VJSON_ARCHIVE} pthread dl)

  if (BUILD_SERVER_DEBUG)
    add_executable (cvmfs_swissknife_debug ${CVMFS_SWISSKNIFE_DEBUG_SOURCES})

    if (LIBCURL_BUILTIN)
      add_dependencies (cvmfs_swissknife_debug libcares libcurl)
    endif (LIBCURL_BUILTIN)

    if (SQLITE3_BUILTIN)
      add_dependencies (cvmfs_swissknife_debug sqlite3)
    endif (SQLITE3_BUILTIN)

    if (ZLIB_BUILTIN)
      add_dependencies (cvmfs_swissknife_debug zlib)
    endif (ZLIB_BUILTIN)

    set_target_properties (cvmfs_swissknife_debug PROPERTIES COMPILE_FLAGS "${CVMFS_SWISSKNIFE_DEBUG_CFLAGS}" LINK_FLAGS "${CVMFS_SWISSKNIFE_LD_FLAGS}")

    # link the stuff (*_LIBRARIES are dynamic link libraries)
    target_link_libraries (cvmfs_swissknife_debug  ${CVMFS_SWISSKNIFE_LIBS} ${SQLITE3_LIBRARY} ${CURL_LIBRARIES} ${ZLIB_LIBRARIES} ${OPENSSL_LIBRARIES} ${LIBCURL_ARCHIVE} ${CARES_ARCHIVE} ${SQLITE3_ARCHIVE} ${ZLIB_ARCHIVE} ${RT_LIBRARY} ${VJSON_ARCHIVE} pthread dl)
  endif (BUILD_SERVER_DEBUG)
endif (BUILD_SERVER)

#
# installation
#

if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR "lib")
endif()

if (BUILD_CVMFS)
  install (
    TARGETS      cvmfs2 cvmfs_fsck
    RUNTIME
    DESTINATION    bin
  )

  install (
    TARGETS      cvmfs_fuse cvmfs_fuse_debug
    LIBRARY
    DESTINATION    ${CMAKE_INSTALL_LIBDIR}
  )

  install (
    FILES      cvmfs_talk cvmfs_config
    DESTINATION    bin
    PERMISSIONS    OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )

endif (BUILD_CVMFS)

if (BUILD_LIBCVMFS)
  install (
    FILES      ${CMAKE_CURRENT_BINARY_DIR}/libcvmfs.a
    DESTINATION    ${CMAKE_INSTALL_LIBDIR}
  )
  install (
    FILES                   libcvmfs.h
    DESTINATION             include
  )
endif (BUILD_LIBCVMFS)

if (BUILD_SERVER)
  install (
    TARGETS     cvmfs_swissknife
    RUNTIME
    DESTINATION    bin
  )

  if (BUILD_SERVER_DEBUG)
    install (
      TARGETS     cvmfs_swissknife_debug
      RUNTIME
      DESTINATION   bin
    )
  endif (BUILD_SERVER_DEBUG)

  install (
    FILES      cvmfs_server
    DESTINATION    bin
    PERMISSIONS    OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )

        install(
                FILES           cvmfs_server_hooks.sh.demo
                DESTINATION     "/etc/cvmfs"
                PERMISSIONS     OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        )

endif (BUILD_SERVER)
