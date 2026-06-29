import json
import os
import time

TOPIC = {
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

DB_FILE = "/config/mda26.json"

fingerprints = {}
events = []

door_open = False
door_open_time = 0

sync = set()

remove_retries = {}


# -------------------------

def load():
    global fingerprints, events

    if not os.path.exists(DB_FILE):
        save()
        return

    with open(DB_FILE, "r") as f:
        data = json.load(f)

    fingerprints = data.get("fingerprints", {})
    events = data.get("events", [])


def save():
    with open(DB_FILE, "w") as f:
        json.dump(
            {
                "fingerprints": fingerprints,
                "events": events
            },
            f,
            indent=4
        )


def add_event(event):
    event["time"] = int(time.time())
    events.append(event)

    # prevent infinite growth
    if len(events) > 1000:
        del events[:-1000]

    save()


# -------------------------

@time_trigger("startup")
def startup():
    logger.debug("MDA26 server initialized")
    load()


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

            logger.info("Door opened")

        door_open = True


    elif payload == "closed":

        if door_open:
            logger.info("Door closed")

        door_open = False
        door_open_time = 0
        door_alert_sent = False


def get_door_alert_time():
    value = state.get("input_number.door_alert_time")

    if value is None:
        return 60  # fallback

    return float(value)

@time_trigger("period(/10)")
def check_door():

    global door_alert_sent

    if not door_open:
        return

    alert_time = get_door_alert_time()

    elapsed = time.time() - door_open_time

    if elapsed >= alert_time and not door_alert_sent:

        logger.warning(
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

    logger.info(
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

    logger.debug("ESP fingerprint sync: %s", payload)

    try:
        fid = int(payload)
    except:
        logger.error("Invalid fingerprint ID: %s", payload)
        return


    if fid == -1:
        # ESP finished sending list.
        # Remove IDs not found in ESP.
        logger.debug("Revising fingerprint missing IDs")

        missing = [
            x for x in fingerprints
            if int(x) not in sync
        ]

        for x in missing:
            logger.debug("Removing missing fingerprint ID: %s", x)
            del fingerprints[x]

        sync.clear()
        save()
        return


    sync.add(fid)


    # already exists
    if str(fid) in fingerprints:
        logger.debug("Already existed fingerprint ID: %s", payload)
        return


    # new fingerprint
    fingerprints[str(fid)] = {
        "name": f"Fingerprint {fid}",
        "created": int(time.time())
    }

    logger.debug("Added missing fingerprint ID: %s", payload)

    add_event({
        "type": "enrolled_sync",
        "id": fid
    })


@mqtt_trigger("mda26/fingerprint/scan")
def mqtt_scan(payload=None):

    try:
        fid = int(payload)

    except:
        logger.error("Invalid fingerprint ID: %s", payload)
        return


    add_event({
        "type": "scan",
        "id": fid
    })


    if str(fid) in fingerprints:

        name = fingerprints[str(fid)]["name"]

        message = f"Door opened by {name}"

        logger.info(message)


    else:

        message = f"Door opened by unknown fingerprint ID: {fid}"

        logger.warning(message)


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
        logger.error("Invalid fingerprint ID: %s", payload)
        return


    fingerprints[str(fid)] = {
        "name": f"Fingerprint {fid}",
        "created": int(time.time())
    }

    logger.debug("Enrolled new fingerprint ID: %s", payload)


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
        logger.error("Invalid fingerprint ID: %s", payload)
        return


    fingerprints.pop(str(fid), None)

    remove_retries.pop(fid, None)

    add_event({
        "type": "removed",
        "id": fid
    })

    logger.debug("Successfully removed fingerprint ID: %s", payload)


    save()



@mqtt_trigger("mda26/fingerprint/remove_failed")
def mqtt_remove_failed(payload=None):

    try:
        fid = int(payload)
    except:
        return


    remove_retries[fid] = remove_retries.get(fid, 0) + 1


    if remove_retries[fid] < 3:

        logger.debug("Retrying remove fingerprint ID: %s", payload)

        # retry
        mqtt.publish(
            TOPIC["fp_remove"],
            str(fid)
        )

    else:

        add_event({
            "type": "remove_failed",
            "id": fid,
            "attempts": remove_retries[fid]
        })

        logger.debug("Could not remove fingerprint ID: %s", payload)

        remove_retries.pop(fid, None)



# -------------------------
# SERVICES
# -------------------------

@service
def mda26_open():

    mqtt.publish(
        TOPIC["door"],
        "open"
    )


@service
def mda26_close():

    mqtt.publish(
        TOPIC["door"],
        "close"
    )


@service
def mda26_open_time(milliseconds):

    mqtt.publish(
        TOPIC["door"],
        str(milliseconds)
    )


@service
def mda26_enroll():

    mqtt.publish(
        TOPIC["fp_cmd"],
        "enroll"
    )


@service
def mda26_remove(id):

    mqtt.publish(
        TOPIC["fp_remove"],
        str(id)
    )