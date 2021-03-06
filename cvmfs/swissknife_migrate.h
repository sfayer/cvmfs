/**
 * This file is part of the CernVM File System.
 */

#ifndef CVMFS_SWISSKNIFE_MIGRATE_H_
#define CVMFS_SWISSKNIFE_MIGRATE_H_

#include "swissknife.h"

#include <map>
#include <vector>
#include <string>

#include "hash.h"
#include "util.h"
#include "util_concurrency.h"
#include "catalog.h"
#include "upload.h"
#include "atomic.h"

namespace catalog {
  class WritableCatalog;
}

namespace swissknife {

class CommandMigrate : public Command {
 protected:
  struct CatalogStatistics {
    unsigned int max_row_id;
    unsigned int entry_count;

    unsigned int hardlink_group_count;
    unsigned int aggregated_linkcounts;

    double       migration_time;

    std::string root_path;
  };

  class CatalogStatisticsList : protected std::vector<CatalogStatistics>,
                                public    Lockable {
    friend class CommandMigrate;

   public:
    inline void Insert(const CatalogStatistics &statistics) {
      LockGuard<CatalogStatisticsList> lock(this);
      this->push_back(statistics);
    }
  };

 public:
  struct PendingCatalog;
  typedef std::vector<PendingCatalog *> PendingCatalogList;
  struct PendingCatalog {
    PendingCatalog(const catalog::Catalog *old_catalog = NULL) :
      success(false),
      old_catalog(old_catalog),
      new_catalog(NULL) {}
    virtual ~PendingCatalog();

    inline const std::string root_path() const {
      return old_catalog->path().ToString();
    }
    inline const bool IsRoot() const { return old_catalog->IsRoot(); }

    bool                              success;

    const catalog::Catalog           *old_catalog;
    catalog::WritableCatalog         *new_catalog;

    PendingCatalogList                nested_catalogs;
    Future<catalog::DirectoryEntry>   root_entry;
    Future<catalog::DeltaCounters>    nested_statistics;

    CatalogStatistics                 statistics;

    Future<hash::Any>                 new_catalog_hash;
  };

  class PendingCatalogMap : public std::map<std::string, const PendingCatalog*>,
                            public Lockable {};

  class MigrationWorker : public ConcurrentWorker<MigrationWorker> {
   public:
    typedef CommandMigrate::PendingCatalog* expected_data;
    typedef CommandMigrate::PendingCatalog* returned_data;

    struct worker_context {
      worker_context(const std::string  &temporary_directory,
                     const bool          fix_nested_catalog_transitions,
                     const bool          analyze_file_linkcounts,
                     const bool          collect_catalog_statistics,
                     const uid_t         uid,
                     const gid_t         gid) :
        temporary_directory(temporary_directory),
        fix_nested_catalog_transitions(fix_nested_catalog_transitions),
        analyze_file_linkcounts(analyze_file_linkcounts),
        collect_catalog_statistics(collect_catalog_statistics),
        uid(uid),
        gid(gid) {}
      const std::string  temporary_directory;
      const bool         fix_nested_catalog_transitions;
      const bool         analyze_file_linkcounts;
      const bool         collect_catalog_statistics;
      const uid_t        uid;
      const gid_t        gid;
    };

   public:
    MigrationWorker(const worker_context *context);
    virtual ~MigrationWorker();

    void operator()(const expected_data &data);

   protected:
    bool CreateNewEmptyCatalog            (PendingCatalog *data) const;
    bool CheckDatabaseSchemaCompatibility (PendingCatalog *data) const;
    bool AttachOldCatalogDatabase         (PendingCatalog *data) const;
    bool StartDatabaseTransaction         (PendingCatalog *data) const;
    bool MigrateFileMetadata              (PendingCatalog *data) const;
    bool AnalyzeFileLinkcounts            (PendingCatalog *data) const;
    bool MigrateNestedCatalogReferences   (PendingCatalog *data) const;
    bool FixNestedCatalogTransitionPoints (PendingCatalog *data) const;
    bool GenerateCatalogStatistics        (PendingCatalog *data) const;
    bool FindRootEntryInformation         (PendingCatalog *data) const;
    bool CommitDatabaseTransaction        (PendingCatalog *data) const;
    bool CollectAndAggregateStatistics    (PendingCatalog *data) const;
    bool DetachOldCatalogDatabase         (PendingCatalog *data) const;
    bool CleanupNestedCatalogs            (PendingCatalog *data) const;

   private:
    const std::string  temporary_directory_;
    const bool         fix_nested_catalog_transitions_;
    const bool         analyze_file_linkcounts_;
    const bool         collect_catalog_statistics_;
    const uid_t        uid_;
    const gid_t        gid_;

    StopWatch               migration_stopwatch_;
  };

 public:
  CommandMigrate();
  ~CommandMigrate() { };
  std::string GetName() { return "migrate"; };
  std::string GetDescription() {
    return "CernVM-FS catalog repository migration \n"
      "This command migrates the whole catalog structure of a given repository";
  };
  ParameterList GetParams();

  int Main(const ArgumentList &args);

  static void FixNestedCatalogTransitionPoint(
                          catalog::DirectoryEntry &mountpoint,
                    const catalog::DirectoryEntry &nested_root);
  static const catalog::DirectoryEntry& GetNestedCatalogMarkerDirent();

 protected:
  void CatalogCallback(const catalog::Catalog* catalog,
                       const hash::Any&        catalog_hash,
                       const unsigned          tree_level);
  void MigrationCallback(PendingCatalog *const &data);
  void UploadCallback(const upload::SpoolerResult &result);

  void ConvertCatalogsRecursively(PendingCatalog *catalog);
  bool RaiseFileDescriptorLimit() const;
  bool ConfigureSQLite() const;
  void AnalyzeCatalogStatistics() const;

  bool GenerateNestedCatalogMarkerChunk();
  void CreateNestedCatalogMarkerDirent(const hash::Any &content_hash);

 private:
  unsigned int           file_descriptor_limit_;
  CatalogStatisticsList  catalog_statistics_list_;
  unsigned int           catalog_count_;
  atomic_int32           catalogs_processed_;

  uid_t                  uid_;
  gid_t                  gid_;

  std::string                     temporary_directory_;
  std::string                     nested_catalog_marker_tmp_path_;
  static catalog::DirectoryEntry  nested_catalog_marker_;

  catalog::Catalog const*                        root_catalog_;
  UniquePtr<ConcurrentWorkers<MigrationWorker> > concurrent_migration_;
  UniquePtr<upload::Spooler>                     spooler_;
  PendingCatalogMap                              pending_catalogs_;

  StopWatch  catalog_loading_stopwatch_;
  StopWatch  migration_stopwatch_;
};

}  // namespace swissknife

#endif  // CVMFS_SWISSKNIFE_MIGRATE_H_
