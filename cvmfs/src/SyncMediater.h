/**
 *  The SyncMediator is an intermediate layer between the UnionSync
 *  implementation and the CatalogHandler.
 *  It's main responsibility is to unwind file system intrinsics as
 *  deleting all files in a deleted directory. Furthermore newly
 *  created directories are recursed and all included files are
 *  added as a whole. (Performance improvement)
 *
 *  Furthermore it keeps track of hard link relations. CVMFS defines
 *  hard links as files with the same inode pointing to the same
 *  chunk of data. As we cannot determine an appropriate inode number
 *  while synching, we just keep track of hard link relations itself.
 *  Inodes will be assigned at run time of the CVMFS client according
 *  to these relations.
 *
 *  Another responsibility of this class is the creation and destruction
 *  of nested catalogs. If a .cvmfscatalog magic file is encountered,
 *  either on delete or add, it will be treated as nested catalog change.
 *
 *  SyncItems containing data (e.g. not Directories or symlinks) will
 *  be accumulated and it's contents are compressed at the end of the
 *  synching process en bloc. This gives us the ability to do this
 *  computational intensive task concurrently at the end of the sync
 *  process.
 *
 *  Developed by René Meusel 2011 at CERN
 */

#ifndef CVMFS_SYNC_MEDIATOR_H
#define CVMFS_SYNC_MEDIATOR_H

#include <list>
#include <string>
#include <map>
#include <stack>

#include "compat.h"
#include "WritableCatalogManager.h"
#include "cvmfs_sync.h"
#include "cvmfs_sync_recursion.h"

namespace cvmfs {

class SyncItem;

/**
 *  If we encounter a file with linkcount > 1 it will be added to a HardlinkGroup
 *  After processing all files, the HardlinkGroups are populated with related hardlinks
 *  Assertion: linkcount == HardlinkGroup::hardlinks.size() at the end!!
 */

typedef struct {
	SyncItem *masterFile;
	SyncItemList hardlinks;
} HardlinkGroup;

/**
 *  a mapping of inode number to the related HardlinkGroup
 */
typedef std::map<uint64_t, HardlinkGroup> HardlinkGroupMap;

/**
 *  The SyncMediator refines the input received from a concrete UnionSync object
 *  for example it resolves the insertion and deletion of complete directories by recursing them
 *  It works as a mediator between the union file system and forwards the correct database
 *  commands to the CatalogHandler to sync the changes into the repository
 *  Furthermore it handles the data compression and storage
 */
class SyncMediator {
private:
	typedef std::stack<HardlinkGroupMap> HardlinkGroupMapStack;
	typedef std::list<HardlinkGroup> HardlinkGroupList;
	
private:
	WritableCatalogManager *mCatalogManager;
	std::string mDataDirectory;
	
	/**
	 *  hardlinks are supported as long as they all reside in the SAME directory.
	 *  If a recursion enters a directory, we push an empty HardlinkGroupMap to accommodate the hardlinks of this directory.
	 *  When leaving a directory (i.e. it is completely processed) the stack is popped and the HardlinkGroupMap receives
	 *  further processing. 
	 */
	HardlinkGroupMapStack mHardlinkStack;
	
	/**
	 *  Files and hardlinks are not simply added to the catalogs but kept in a queue
	 *  when committing the changes after recursing the read write branch of the union file system
	 *  the queues will processed en bloc (for parallelization purposes) and afterwards added
	 */
	SyncItemList mFileQueue;
	HardlinkGroupList mHardlinkQueue;
	
	/** if dry run is set no changes to the repository will be made */
	bool mDryRun;
	
	/** if print changeset option is set it will print all changes to the repository to stdout */
	bool mPrintChangeset;
	
public:
	SyncMediator(WritableCatalogManager *catalogManager, const SyncParameters *parameters);
	virtual ~SyncMediator();
	
	/**
	 *  adding an entry to the repository
	 *  this method does further processing: f.e. added directories will be recursed
	 *  to add all its contents
	 *  @param entry the entry to add
	 */
	void add(SyncItem *entry);
	
	/**
	 *  touching an entry in the repository
	 *  @param the entry to touch
	 */
	void touch(SyncItem *entry);
	
	/**
	 *  removing an entry from the repository
	 *  directories will be recursively removed to get rid of its contents
	 *  @param entry the entry to remove
	 */
	void remove(SyncItem *entry);
	
	/**
	 *  meta operation
	 *  basically remove the old entry and add the new one
	 *  @param entry the entry to be replaced by a new one
	 */
	void replace(SyncItem *entry);
	
	/**
	 *  notifier that a new directory was opened for recursion
	 *  @param entry the opened directory
	 */
	void enterDirectory(SyncItem *entry);
	
	/**
	 *  notifier that a directory is fully processed by a recursion
	 *  @param entry the directory which will be left by the recursion
	 */
	void leaveDirectory(SyncItem *entry);
	
	/**
	 *  do any pending processing and commit all changes to the catalogs
	 *  to be called AFTER all recursions are finished
	 */
	void commit();
	
private:
	void addDirectoryRecursively(SyncItem *entry);
	void removeDirectoryRecursively(SyncItem *entry);
	RecursionPolicy addDirectoryCallback(SyncItem *entry);
	
	uint64_t getTemporaryHardlinkGroupNumber(SyncItem *entry) const;
	void insertHardlink(SyncItem *entry);
	void insertExistingHardlink(SyncItem *entry);
	void completeHardlinks(SyncItem *entry);
	
	void addFile(SyncItem *entry);
	void removeFile(SyncItem *entry);
	void touchFile(SyncItem *entry);
	
	void addDirectory(SyncItem *entry);
	void removeDirectory(SyncItem *entry);
	void touchDirectory(SyncItem *entry);
	
	void createNestedCatalog(SyncItem *requestFile);
	void removeNestedCatalog(SyncItem *requestFile);
	
	void leaveAddedDirectory(SyncItem *entry);
	
	void compressAndHashFileQueue();
	void addFileQueueToCatalogs();
   void releaseFileQueue();
	
	inline bool addFileToDatastore(SyncItem *entry, hash::t_sha1 &hash) { return addFileToDatastore(entry, "", hash); }
	bool addFileToDatastore(SyncItem *entry, const std::string &suffix, hash::t_sha1 &hash);
	
	inline HardlinkGroupMap& getHardlinkMap() { return mHardlinkStack.top(); }
	
	void addHardlinkGroups(const HardlinkGroupMap &hardlinks);
  void addHardlinkGroup(const HardlinkGroup &group);
  void cleanupHardlinkGroups(HardlinkGroupMap &hardlinks);
};

}

#endif