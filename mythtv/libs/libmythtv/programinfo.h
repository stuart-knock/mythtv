#ifndef PROGRAMINFO_H_
#define PROGRAMINFO_H_

#include <qsqldatabase.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qmap.h>
#include "scheduledrecording.h"

#define NUMPROGRAMLINES 32

enum MarkTypes {
    MARK_UPDATED_CUT = -3,
    MARK_EDIT_MODE = -2,
    MARK_PROCESSING = -1,
    MARK_CUT_END = 0,
    MARK_CUT_START = 1,
    MARK_BOOKMARK = 2,
    MARK_BLANK_FRAME = 3,
    MARK_COMM_START = 4,
    MARK_COMM_END = 5,
    MARK_GOP_START = 6,
    MARK_KEYFRAME = 7,
    MARK_SCENE_CHANGE = 8,
    MARK_GOP_BYFRAME = 9
};

enum TranscoderStatus {
    TRANSCODE_QUEUED      = 0x01,
    TRANSCODE_RETRY       = 0x02,
    TRANSCODE_FAILED      = 0x03,
    TRANSCODE_LAUNCHED    = 0x04,
    TRANSCODE_STARTED     = 0x05,
    TRANSCODE_FINISHED    = 0x06,
    TRANSCODE_USE_CUTLIST = 0x10,
    TRANSCODE_STOP        = 0x20,
    TRANSCODE_FLAGS       = 0xF0
};

enum FlagMask {
    FL_COMMFLAG  = 0x01,
    FL_CUTLIST   = 0x02,
    FL_AUTOEXP   = 0x04,
    FL_EDITING   = 0x08,
    FL_BOOKMARK  = 0x10,
};

enum RecStatusType {
    rsDeleted = -5,
    rsStopped = -4,
    rsRecorded = -3,
    rsRecording = -2,
    rsWillRecord = -1,
    rsUnknown = 0,
    rsManualOverride = 1,
    rsPreviousRecording = 2,
    rsCurrentRecording = 3,
    rsEarlierShowing = 4,
    rsTooManyRecordings = 5,
    rsCancelled = 6,
    rsConflict = 7,
    rsLaterShowing = 8,
    //rsUnused = 9,
    rsOverlap = 10,
    rsLowDiskSpace = 11,
    rsTunerBusy = 12
};

class QSqlDatabase;

class ProgramInfo
{
  public:
    ProgramInfo();
    ProgramInfo(const ProgramInfo &other);
    ~ProgramInfo();

    ProgramInfo& operator=(const ProgramInfo &other);

    // returns 0 for one-time, 1 for weekdaily, 2 for weekly
    int IsProgramRecurring(void);

    // checks for duplicates according to dupmethod
    bool IsSameProgram(const ProgramInfo& other) const;
    // checks chanid, start/end times, sourceid, cardid, inputid.
    bool IsSameTimeslot(const ProgramInfo& other) const;
    // checks chanid, start/end times, sourceid
    bool IsSameProgramTimeslot(const ProgramInfo& other) const;

    void Save(QSqlDatabase *db);

    RecordingType GetProgramRecordingStatus(QSqlDatabase *db);
    QString GetProgramRecordingProfile(QSqlDatabase *db);
    bool AllowRecordingNewEpisodes(QSqlDatabase *db);
    int GetChannelRecPriority(QSqlDatabase *db, const QString &chanid);
    int GetRecordingTypeRecPriority(RecordingType type);

    void ApplyRecordStateChange(QSqlDatabase *db, RecordingType newstate);
    void ApplyRecordTimeChange(QSqlDatabase *db, 
                               const QDateTime &newstartts,
                               const QDateTime &newendts);
    void ApplyRecordRecPriorityChange(QSqlDatabase *db, int);
    void ApplyRecordRecGroupChange(QSqlDatabase *db,
                               const QString &newrecgroup);
    void ToggleRecord(QSqlDatabase *dB);

    ScheduledRecording* GetScheduledRecording(QSqlDatabase *db) 
    {
        GetProgramRecordingStatus(db);
        return record;
    };

    int getRecordID(QSqlDatabase *db)
    {
        GetProgramRecordingStatus(db);
        recordid = record->getRecordID();
        return recordid;
    }

    void StartedRecording(QSqlDatabase *db);
    void FinishedRecording(QSqlDatabase* db, bool prematurestop);

    QGridLayout* DisplayWidget(QWidget *parent = NULL);

    QString GetRecordBasename(void);
    QString GetRecordFilename(const QString &prefix);

    QString MakeUniqueKey(void) const;

    int CalculateLength(void);

    void ToStringList(QStringList &list);
    void FromStringList(QStringList &list, int offset);
    void FromStringList(QStringList &list, QStringList::iterator &it);
    void ToMap(QSqlDatabase *db, QMap<QString, QString> &progMap);

    void SetBookmark(long long pos, QSqlDatabase *db);
    long long GetBookmark(QSqlDatabase *db);
    bool IsEditing(QSqlDatabase *db);
    void SetEditing(bool edit, QSqlDatabase *db);
    bool IsCommFlagged(QSqlDatabase *db);
    // 1 = flagged, 2 = processing
    void SetCommFlagged(int flag, QSqlDatabase *db);
    bool IsCommProcessing(QSqlDatabase *db);
    void SetAutoExpire(bool autoExpire, QSqlDatabase *db);
    bool GetAutoExpireFromRecorded(QSqlDatabase *db);

    void GetCutList(QMap<long long, int> &delMap, QSqlDatabase *db);
    void SetCutList(QMap<long long, int> &delMap, QSqlDatabase *db);

    void SetBlankFrameList(QMap<long long, int> &frames, QSqlDatabase *db,
                           long long min_frame = -1, long long max_frame = -1);
    void GetBlankFrameList(QMap<long long, int> &frames, QSqlDatabase *db);

    void SetCommBreakList(QMap<long long, int> &frames, QSqlDatabase *db);
    void GetCommBreakList(QMap<long long, int> &frames, QSqlDatabase *db);

    void ClearMarkupMap(QSqlDatabase *db, int type = -100,
                      long long min_frame = -1, long long max_frame = -1);
    void SetMarkupMap(QMap<long long, int> &marks, QSqlDatabase *db,
                      int type = -100,
                      long long min_frame = -1, long long max_frame = -1);
    void GetMarkupMap(QMap<long long, int> &marks, QSqlDatabase *db,
                      int type, bool mergeIntoMap = false);
    bool CheckMarkupFlag(int type, QSqlDatabase *db);
    void SetMarkupFlag(int type, bool processing, QSqlDatabase *db);
    void GetPositionMap(QMap<long long, long long> &posMap, int type,
                        QSqlDatabase *db);
    void SetPositionMap(QMap<long long, long long> &posMap, int type,
                        QSqlDatabase *db,
                        long long min_frame = -1, long long max_frame = -1);

    void DeleteHistory(QSqlDatabase *db);
    QString RecTypeChar(void);
    QString RecTypeText(void);
    QString RecStatusChar(void);
    QString RecStatusText(void);
    QString RecStatusDesc(void);
    void FillInRecordInfo(vector<ProgramInfo *> &reclist);
    void EditScheduled(QSqlDatabase *db);
    void EditRecording(QSqlDatabase *db);

    int getProgramFlags(QSqlDatabase *db);

    static void GetProgramListByQuery(QSqlDatabase *db,
                                        QPtrList<ProgramInfo> *proglist,
                                        const QString &where);
    static void GetProgramRangeDateTime(QSqlDatabase *db,
                                        QPtrList<ProgramInfo> *proglist, 
                                        const QString &channel, 
                                        const QString &ltime, 
                                        const QString &rtime);
    static ProgramInfo *GetProgramAtDateTime(QSqlDatabase *db, 
                                             const QString &channel, 
                                             const QString &time);
    static ProgramInfo *GetProgramAtDateTime(QSqlDatabase *db,
                                             const QString &channel, 
                                             QDateTime &dtime);
    static ProgramInfo *GetProgramFromRecorded(QSqlDatabase *db,
                                               const QString &channel, 
                                               const QString &starttime);
    static ProgramInfo *GetProgramFromRecorded(QSqlDatabase *db,
                                               const QString &channel, 
                                               QDateTime &dtime);
    int SecsTillStart() const { return QDateTime::currentDateTime().secsTo(startts); }

    QString title;
    QString subtitle;
    QString description;
    QString category;

    QString chanid;
    QString chanstr;
    QString chansign;
    QString channame;
    int recpriority;
    QString recgroup;
    int chancommfree;

    QString pathname;
    long long filesize;
    QString hostname;

    QDateTime startts;
    QDateTime endts;
    QDateTime recstartts;
    QDateTime recendts;

    bool repeat;

    int spread;
    int startCol;

    int override;
    RecStatusType recstatus;
    RecStatusType savedrecstatus;
    int numconflicts;
    int conflictpriority;
    int reactivate;             // 0 = not requested
                                // 1 = requested, pending
                                // 2 = requested, replaced
                                // -1 = reactivated
    int recordid;
    RecordingType rectype;
    RecordingDupInType dupin;
    RecordingDupMethodType dupmethod;

    int sourceid;
    int inputid;
    int cardid;
    bool shareable;
    bool conflictfixed;

    QString schedulerid;

    int programflags;

private:
    void handleRecording(QSqlDatabase *db);
    void handleNotRecording(QSqlDatabase *db);
    void setOverride(QSqlDatabase *db, int override);

    class ScheduledRecording* record;
};

class PGInfoCon
{
  public:
    PGInfoCon();
    ~PGInfoCon();
    
    typedef QPtrListIterator<ProgramInfo> PGInfoConIt;
    
    void updateAll(const vector<ProgramInfo *> &pginfoList);
    bool update(const ProgramInfo &pginfo);
    int count();
    void clear();
   
    int selectedIndex();
    void setSelected(int delta = 1);
    bool getSelected(ProgramInfo &pginfo);
    
    PGInfoConIt iterator();
  private:
    void init();
    void checkSetIndex();
    ProgramInfo *find(const ProgramInfo &pginfo);
    ProgramInfo *findFromKey(const QString &key);
    ProgramInfo *findNearest(const ProgramInfo &pginfo);
     
    QPtrList<ProgramInfo> allData;
    int selectedPos;
};

#endif
