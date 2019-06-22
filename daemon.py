#!/usr/bin/env python

import os
import json
import serial
import random
from twitter import Api

s = serial.Serial(port="/dev/cu.usbmodem14101", baudrate=9600)

# Either specify a set of keys here or use os.getenv('CONSUMER_KEY') style
# assignment:

CONSUMER_KEY = 'PKyfBifadEMRTI7XXSDh5qGVo'
# CONSUMER_KEY = os.getenv("CONSUMER_KEY", None)
CONSUMER_SECRET = 'EdlQk9FYAhVwlwOcDct2LL5ktki3ogzd0WUOFLpdxPcyTEaNVn'
# CONSUMER_SECRET = os.getenv("CONSUMER_SECRET", None)
ACCESS_TOKEN = '381769239-uxCiTqylKO45ItcMnOZ0BigFYBQzlNdG7CxXRQK0'
# ACCESS_TOKEN = os.getenv("ACCESS_TOKEN", None)
ACCESS_TOKEN_SECRET = 'nhCPDFF4HkOpsBU3ns1jocjHXVpX8HYvGcQffJdgxM99c'
# ACCESS_TOKEN_SECRET = os.getenv("ACCESS_TOKEN_SECRET", None)

# Users to watch for should be a list. This will be joined by Twitter and the
# data returned will be for any tweet mentioning:
# @twitter *OR* @twitterapi *OR* @support.
USERS = ['#astrophysics',
          '#rocket',
          '#iss',
          '#sky',
          '#physics',
          '#falcon',
          '#apollo',
          '#spaceexploration',
          '#nebula',
          '#astrophotography',
          '#planets',
          '#telescope',
          '#nasa',
          '#spacestation',
          '#milkyway',
          '#spacecraft',
          '#outerspace',
          '#NASAsilab',
          'iste2019']

# Languages to filter tweets by is a list. This will be joined by Twitter
# to return data mentioning tweets only in the english language.
LANGUAGES = ['en']

# Since we're going to be using a streaming endpoint, there is no need to worry
# about rate limits.
api = Api(CONSUMER_KEY,
          CONSUMER_SECRET,
          ACCESS_TOKEN,
          ACCESS_TOKEN_SECRET)


def main():
    #with open('output.txt', 'a') as f:
        # api.GetStreamFilter will return a generator that yields one status
        # message (i.e., Tweet) at a time as a JSON dictionary.
    for line in api.GetStreamFilter(track=USERS, languages=LANGUAGES):
        r = lambda: random.randint(0,255)
        s.write(str.encode('1'))
        print(json.dumps(line))
        print('\n')

if __name__ == '__main__':
    main()
