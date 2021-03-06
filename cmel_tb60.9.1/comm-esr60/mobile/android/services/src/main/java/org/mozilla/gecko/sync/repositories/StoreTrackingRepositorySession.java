/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.sync.repositories;

import java.util.Collection;
import java.util.Iterator;

import org.mozilla.gecko.background.common.log.Logger;
import org.mozilla.gecko.sync.SyncException;
import org.mozilla.gecko.sync.repositories.delegates.RepositorySessionFinishDelegate;
import org.mozilla.gecko.sync.repositories.domain.Record;

public abstract class StoreTrackingRepositorySession extends RepositorySession {
  private static final String LOG_TAG = "StoreTrackSession";
  protected StoreTracker storeTracker;

  protected static StoreTracker createStoreTracker() {
    return new HashSetStoreTracker();
  }

  public StoreTrackingRepositorySession(Repository repository) {
    super(repository);
  }

  @Override
  public void begin() throws SyncException {
    super.sharedBegin();
    // Or do this in your own subclass.
    storeTracker = createStoreTracker();
  }

  @Override
  protected synchronized void trackGUID(String guid) {
    if (this.storeTracker == null) {
      throw new IllegalStateException("Store tracker not yet initialized!");
    }
    this.storeTracker.trackRecordForExclusion(guid);
  }

  @Override
  public synchronized void untrackGUID(String guid) {
    if (this.storeTracker == null) {
      throw new IllegalStateException("Store tracker not yet initialized!");
    }
    this.storeTracker.untrackStoredForExclusion(guid);
  }

  @Override
  public synchronized void untrackGUIDs(Collection<String> guids) {
    if (this.storeTracker == null) {
      throw new IllegalStateException("Store tracker not yet initialized!");
    }
    if (guids == null) {
      return;
    }
    for (String guid : guids) {
      this.storeTracker.untrackStoredForExclusion(guid);
    }
  }

  public void trackRecord(Record record) {
    // Future: we care about the timestampâ€¦
    trackGUID(record.guid);
  }

  protected void untrackRecord(Record record) {
    Logger.debug(LOG_TAG, "Un-tracking record " + record.guid + ".");
    untrackGUID(record.guid);
  }

  @Override
  public Iterator<String> getTrackedRecordIDs() {
    if (this.storeTracker == null) {
      throw new IllegalStateException("Store tracker not yet initialized!");
    }
    return this.storeTracker.recordsTrackedForExclusion();
  }

  @Override
  public void abort(RepositorySessionFinishDelegate delegate) {
    this.storeTracker = null;
    super.abort(delegate);
  }

  @Override
  public void finish(RepositorySessionFinishDelegate delegate) throws InactiveSessionException {
    super.finish(delegate);
    this.storeTracker = null;
  }
}
