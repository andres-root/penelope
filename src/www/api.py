
from .models import Tweet
import tweepy


class StreamListener(tweepy.StreamListener):

    def __init__(self):
        super().__init__()
        self.counter = 0
        self.limit = 1

    def on_status(self, status):
        if self.counter < self.limit:
            print(status.text)
            self.counter += 1
            tweet = Tweet(name='test', user='@testusername', text=status.text)
            tweet.save()

        else:
            return False

    def on_error(self, status_code):
        return False

    def on_timeout(self):
        return False


class Twitter(object):

    def __init__(self):
        self.consumer_key = 'j6yrXInZz1EU5S72WSuwgBUln'
        self.consumer_secret = 'Dy5JfkHrVgdJGLdLCTsjVOC2HDEwFplJCKZyymxvQe1F5yYpLY'
        self.access_token = '54296117-B4CEjQS35kllLzXUCMLJDHZzgd6Q3o0EMF4iwKwc2'
        self.access_token_secret = 'PoRvZpY8sHCHV6rz5uGysWzV6Mc2pVfrQJx8eZoprAJUV'
        self.api_auth = None
        self.api = None
        self.api_stream = None

    def auth(self):
        self.api_auth = tweepy.OAuthHandler(self.consumer_key, self.consumer_secret)
        self.api_auth.set_access_token(self.access_token, self.access_token_secret)
        self.api = tweepy.API(self.api_auth)
        return self.api

    def stream(self, topic):
        self.auth()
        self.api_stream = tweepy.Stream(auth=self.api.auth, listener=StreamListener())
        self.api_stream.filter(track=[topic])
