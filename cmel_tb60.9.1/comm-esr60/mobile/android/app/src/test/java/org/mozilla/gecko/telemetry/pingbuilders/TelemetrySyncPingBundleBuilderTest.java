/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */

package org.mozilla.gecko.telemetry.pingbuilders;

import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;

import org.json.JSONException;
import org.json.simple.JSONArray;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mozilla.gecko.background.testhelpers.TestRunner;
import org.mozilla.gecko.sync.ExtendedJSONObject;
import org.mozilla.gecko.sync.telemetry.TelemetryContract;
import org.mozilla.gecko.telemetry.TelemetryOutgoingPing;
import org.mozilla.gecko.telemetry.TelemetryPing;
import org.mozilla.gecko.telemetry.stores.TelemetryJSONFilePingStore;
import org.mozilla.gecko.telemetry.stores.TelemetryPingStore;

import java.io.IOException;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Set;

import static org.junit.Assert.*;

@RunWith(TestRunner.class)
public class TelemetrySyncPingBundleBuilderTest {
    public static class MockTelemetryPingStore extends TelemetryPingStore {
        public MockTelemetryPingStore() {
            super("default");
        }

        // Stable ordering for the sake of easier testing.
        private LinkedHashMap<String, TelemetryPing> pings = new LinkedHashMap<>();

        @Override
        public List<TelemetryPing> getAllPings() {
            return new ArrayList<>(pings.values());
        }

        @Override
        public int getCount() {
            return pings.size();
        }

        @Override
        public void storePing(TelemetryPing ping) throws IOException {
            pings.put(ping.getDocID(), ping);
        }

        @Override
        public void maybePrunePings() {}

        @Override
        public void onUploadAttemptComplete(Set<String> successfulRemoveIDs) {
            for (String id : successfulRemoveIDs) {
                pings.remove(id);
            }
        }

        @Override
        public Set<String> getStoredIDs() {
            return pings.keySet();
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {

        }

        public final Parcelable.Creator<TelemetryJSONFilePingStore> CREATOR = new Parcelable.Creator<TelemetryJSONFilePingStore>() {
            @Override
            public TelemetryJSONFilePingStore createFromParcel(Parcel source) {
                return null;
            }

            @Override
            public TelemetryJSONFilePingStore[] newArray(int size) {
                return new TelemetryJSONFilePingStore[0];
            }
        };
    }

    private TelemetrySyncPingBundleBuilder builder;
    private MockTelemetryPingStore syncPings = new MockTelemetryPingStore();
    private MockTelemetryPingStore eventPings = new MockTelemetryPingStore();

    @Before
    public void setUp() throws Exception {
        builder = new TelemetrySyncPingBundleBuilder();
        builder.setReason(TelemetrySyncPingBundleBuilder.UPLOAD_REASON_SCHEDULE);
        syncPings = new MockTelemetryPingStore();
        eventPings = new MockTelemetryPingStore();
    }

    @Test
    public void testGeneralShape() throws Exception {
        builder.setSyncStore(syncPings);
        builder.setSyncEventStore(eventPings);
        builder.setDeviceID("device-id-1");
        builder.setUID("uid-1");

        TelemetryOutgoingPing outgoingPing = builder.build();

        assertTrue(outgoingPing.getPayload().containsKey("application"));
        assertTrue(outgoingPing.getPayload().containsKey("payload"));
        assertTrue(outgoingPing.getPayload().containsKey("id"));
        assertEquals("sync", outgoingPing.getPayload().getString("type"));
        assertEquals(Integer.valueOf(4), outgoingPing.getPayload().getIntegerSafely("version"));

        // Test application key.
        ExtendedJSONObject application = outgoingPing.getPayload().getObject("application");
        assertEquals("Mozilla", application.getString("vendor"));
        assertTrue(application.containsKey("architecture"));
        assertTrue(application.containsKey("platformVersion"));
        assertTrue(application.containsKey("displayVersion"));
        assertTrue(application.containsKey("version"));
        assertTrue(application.containsKey("name"));
        assertTrue(application.containsKey("channel"));
        assertTrue(application.containsKey("buildId"));
        assertTrue(application.containsKey("xpcomAbi"));


        // Test general shape of payload. Expecting {"why":"schedule", "version": 1,
        // "os": {"name": "Android", "version": "<version>", "locale": "<locale>"},
        // "deviceID": <Hashed Device ID>, "uid": <Hashed UID>}.
        ExtendedJSONObject payload = outgoingPing.getPayload().getObject("payload");
        assertEquals(5, payload.keySet().size());
        assertEquals("schedule", payload.getString("why"));
        assertEquals(Integer.valueOf(1), payload.getIntegerSafely("version"));
        assertEquals(payload.getString("uid"), "uid-1");
        assertEquals(payload.getString("deviceID"), "device-id-1");
        // Test os key.
        ExtendedJSONObject os = payload.getObject("os");
        assertEquals(3, os.keySet().size());
        assertEquals("Android", os.getString("name"));
        // Going to be different depending on the test environment.
        // Test for presence and type to void random failures.
        assertTrue(os.getIntegerSafely("version") != null);
        // Likely "en-US" in tests, but let's test for presence and type to avoid random failures.
        assertTrue(os.getString("locale") != null);
    }

    @Test
    public void testBundlingOfMultiplePings() throws Exception {
        // Try just one ping first.
        syncPings.storePing(new TelemetrySyncPingBuilder()
                .setRestarted(true)
                .setTook(123L)
                .build()
        );
        builder.setSyncStore(syncPings);

        TelemetryOutgoingPing outgoingPing = builder.build();

        // Ensure we have that one ping.
        ExtendedJSONObject payload = outgoingPing.getPayload().getObject("payload");
        assertEquals("schedule", payload.getString("why"));
        JSONArray syncs = payload.getArray("syncs");
        assertEquals(1, syncs.size());
        assertSync((ExtendedJSONObject) syncs.get(0), 123L, true);

        // Add another ping.
        syncPings.storePing(new TelemetrySyncPingBuilder()
                .setRestarted(false)
                .setTook(321L)
                .build()
        );
        builder.setSyncStore(syncPings);

        // We should have two pings now.
        outgoingPing = builder.build();
        syncs = outgoingPing.getPayload()
                .getObject("payload")
                .getArray("syncs");
        assertEquals(2, syncs.size());
        assertSync((ExtendedJSONObject) syncs.get(0), 123L, true);
        assertSync((ExtendedJSONObject) syncs.get(1), 321L, false);

        // And add an event ping!
        Bundle event = new Bundle();
        event.putLong(TelemetryContract.KEY_EVENT_TIMESTAMP, 123456L);
        event.putString(TelemetryContract.KEY_EVENT_CATEGORY, "sync");
        event.putString(TelemetryContract.KEY_EVENT_OBJECT, "object");
        event.putString(TelemetryContract.KEY_EVENT_METHOD, "method");
        event.putString(TelemetryContract.KEY_EVENT_VALUE, "value");
        Bundle extra = new Bundle();
        extra.putString("extra-key", "extra-value");
        event.putBundle(TelemetryContract.KEY_EVENT_EXTRA, extra);
        eventPings.storePing(new TelemetrySyncEventPingBuilder()
                .fromEventTelemetry(event)
                .build()
        );
        builder.setSyncEventStore(eventPings);

        // We should have three pings now.
        outgoingPing = builder.build();
        JSONArray events = outgoingPing.getPayload()
                .getObject("payload")
                .getArray("events");
        assertEquals(1, events.size());
        Assert.assertEquals("[[123456,\"sync\",\"method\",\"object\",\"value\",{\"extra-key\":\"extra-value\"}]]",
                events.toJSONString());
    }

    private void assertSync(ExtendedJSONObject sync, long took, boolean restarted) throws JSONException {
        assertEquals(Long.valueOf(took), sync.getLong("took"));

        // Test that 'when' timestamp looks generally sane.
        final long now = System.currentTimeMillis();
        final long yearAgo = now - 1000L * 60 * 60 * 24 * 365;
        assertTrue(sync.getLong("when") > yearAgo);
        assertTrue(sync.getLong("when") <= now);

        if (restarted) {
            assertEquals(true, sync.getBoolean("restarted"));
        } else {
            assertFalse(sync.containsKey("restarted"));
        }

    }
}