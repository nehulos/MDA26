import json
import os
import time

topic = TOPIC = {
    "status": "mda26/status",
    "door": "mda26/door",

    "fp_scan": "mda26/fingerprint/scan",
    "fp_exists": "mda26/fingerprint/exists",
    "fp_enrolled": "mda26/fingerprint/enrolled",
    "fp_removed": "mda26/fingerprint/removed",
    "fp_remove_failed": "mda26/fingerprint/remove_failed",

    "fp_cmd": "mda26/fingerprint",
    "fp_remove": "mda26/fingerprint/remove",
}

DB_FILE = "pyscript/mda26.json"

fingerprints = {}
events = []

door_open = False
door_open_time = 0

sync = set()

remove_retries = {}


# ------------------------- 


@pyscript_executor
def load_db():
    if not os.path.exists(DB_FILE):
        return {
            "fingerprints": {},
            "events": []
        }

    with open(DB_FILE, "r", encoding="utf-8") as f:
        return json.load(f)


@pyscript_executor
def save_db(data):
    os.makedirs(os.path.dirname(DB_FILE), exist_ok=True)

    with open(DB_FILE, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=4)

def save():
    save_db({
        "fingerprints": fingerprints,
        "events": events
    })

def add_event(event):
    event["time"] = int(time.time())
    events.append(event)

    # prevent infinite growth
    if len(events) > 1000:
        del events[:-1000]

    save()


# -------------------------

@time_trigger()
def startup():
    global fingerprints, events

    log.warning("Startup")

    db = load_db()

    fingerprints = db.get("fingerprints", {})
    events = db.get("events", {})

    log.warning("Loaded")


#-------------------------------------------------------------#

DOOR_ALERT_TIME = 60  # seconds

door_open = False
door_open_time = 0
door_alert_sent = False


@mqtt_trigger("mda26/status")
def mqtt_status(payload=None):
    global door_open, door_open_time, door_alert_sent

    if payload == "open":

        if not door_open:
            door_open_time = time.time()
            door_alert_sent = False

            log.warning("Door opened")

        door_open = True


    elif payload == "closed":

        if door_open:
            log.warning("Door closed")

        door_open = False
        door_open_time = 0
        door_alert_sent = False


def get_door_alert_time():
    value = state.get("input_number.door_alert_time")

    if value is None:
        return 60  # fallback

    return float(value)

@time_trigger("period(2025-01-01 00:00:00, 10s)")
def check_door():

    global door_alert_sent

    if not door_open:
        return

    alert_time = get_door_alert_time()

    elapsed = time.time() - door_open_time

    if elapsed >= alert_time and not door_alert_sent:

        log.warning(
            "Door has been open for %s seconds",
            int(elapsed)
        )

        # Notify Home Assistant user
        service.call(
            "notify",
            "persistent_notification",
            {
                "message": "The door has been open for too long!",
                "title": "Door warning"
            }
        )

        door_alert_sent = True

@state_trigger("input_number.door_alert_time")
def alert_time_changed(value=None):

    log.warning(
        "New door alert time: %s seconds",
        value
    )

#-------------------------------------------------------------#

@mqtt_trigger("mda26/fingerprint/exists")
def mqtt_exists(payload=None):
    """
    Synchronization with ESP.

    ESP is source of truth.
    """

    log.warning("ESP fingerprint sync: %s", payload)

    try:
        fid = int(payload)
    except:
        log.error("Invalid fingerprint ID: %s", payload)
        return


    if fid == -1:
        # ESP finished sending list.
        # Remove IDs not found in ESP.
        log.warning("Revising fingerprint missing IDs")

        missing = [
            x for x in fingerprints
            if int(x) not in sync
        ]

        for x in missing:
            log.warning("Removing missing fingerprint ID: %s", x)
            del fingerprints[x]

        sync.clear()
        save()
        return


    sync.add(fid)


    # already exists
    if str(fid) in fingerprints:
        log.warning("Already existed fingerprint ID: %s", payload)
        return


    # new fingerprint
    fingerprints[str(fid)] = {
        "name": f"Fingerprint {fid}",
        "created": int(time.time())
    }

    log.warning("Added missing fingerprint ID: %s", payload)

    add_event({
        "type": "enrolled_sync",
        "id": fid
    })


@mqtt_trigger("mda26/fingerprint/scan")
def mqtt_scan(payload=None):

    try:
        fid = int(payload)

    except:
        log.error("Invalid fingerprint ID: %s", payload)
        return


    add_event({
        "type": "scan",
        "id": fid
    })


    if str(fid) in fingerprints:

        name = fingerprints[str(fid)]["name"]

        message = f"Door opened by {name}"

        log.warning(message)


    else:

        message = f"Door opened by unknown fingerprint ID: {fid}"

        log.warning(message)


    # Notify Home Assistant
    service.call(
        "notify",
        "mobile_app_your_phone",
        {
            "title": "Door opened",
            "message": message
        }
    )


@mqtt_trigger("mda26/fingerprint/enrolled")
def mqtt_enrolled(payload=None):

    try:
        fid = int(payload)
    except:
        log.error("Invalid fingerprint ID: %s", payload)
        return


    fingerprints[str(fid)] = {
        "name": f"Fingerprint {fid}",
        "created": int(time.time())
    }

    log.warning("Enrolled new fingerprint ID: %s", payload)


    add_event({
        "type": "enrolled",
        "id": fid
    })


    save()


@mqtt_trigger("mda26/fingerprint/removed")
def mqtt_removed(payload=None):

    try:
        fid = int(payload)
    except:
        log.error("Invalid fingerprint ID: %s", payload)
        return


    fingerprints.pop(str(fid), None)

    remove_retries.pop(fid, None)

    add_event({
        "type": "removed",
        "id": fid
    })

    log.warning("Successfully removed fingerprint ID: %s", payload)


    save()



@mqtt_trigger("mda26/fingerprint/remove_failed")
def mqtt_remove_failed(payload=None):

    try:
        fid = int(payload)
    except:
        return


    remove_retries[fid] = remove_retries.get(fid, 0) + 1


    if remove_retries[fid] < 3:

        log.warning("Retrying remove fingerprint ID: %s", payload)

        # retry
        mqtt.publish(
            topic = TOPIC["fp_remove"],
            payload = str(fid)
        )

    else:

        add_event({
            "type": "remove_failed",
            "id": fid,
            "attempts": remove_retries[fid]
        })

        log.warning("Could not remove fingerprint ID: %s", payload)

        remove_retries.pop(fid, None)



# -------------------------
# SERVICES
# -------------------------

@service
def mda26_open():

    mqtt.publish(
        topic = TOPIC["door"],
        payload = "open"
    )


@service
def mda26_close():

    mqtt.publish(
        topic = TOPIC["door"],
        payload = "close"
    )


@service
def mda26_open_time(milliseconds):

    mqtt.publish(
        topic = TOPIC["door"],
        payload = str(milliseconds)
    )


@service
def mda26_enroll():

    mqtt.publish(
        topic = TOPIC["fp_cmd"],
        payload = "enroll"
    )


@service
def mda26_remove(id):

    mqtt.publish(
        topic = TOPIC["fp_remove"],
        payload = str(id)
    )
