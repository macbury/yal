#!/usr/bin/env /Library/Frameworks/Python.framework/Versions/3.6/bin/python3
import os
import time
import logging
import paho.mqtt.client as mqtt
from circleci.api import Api
from ruamel.yaml import YAML

logging.basicConfig(
  level=logging.INFO,
  format="[%(asctime)s] %(levelname)s:%(name)s:%(message)s"
)

CONFIG_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'config.yaml')
LOGGER = logging.getLogger('lamp')

LOGGER.info("Loading config: " + CONFIG_PATH)
CONFIG = YAML(typ='safe').load(open(CONFIG_PATH))

circleci = Api(CONFIG['circle_token'])
client = mqtt.Client()

if 'username' in CONFIG:
  client.username_pw_set(CONFIG['username'], CONFIG['password'])
  client.connect(CONFIG['host'], CONFIG['port'], 60)
  client.loop_start()

try:
  while True:
    LOGGER.info("Fetching build status...")
    build = circleci.get_recent_builds()[0]
    status = build['status']
    LOGGER.info("Status is: {}".format(status))
    client.publish(CONFIG['topic'], status, 2, True)
    time.sleep(5)

except KeyboardInterrupt:
  pass
