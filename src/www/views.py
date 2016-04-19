
# from django.shortcuts import render
from django.http import HttpResponse, JsonResponse
from .api import Twitter
from .models import Tweet


def index(request):
    try:
        twitter = Twitter()
        twitter.stream('amor')
        tweets = Tweet.objects.all()
        tweet = tweets[len(tweets) - 1]
        tweet_object = {'name': tweet.name, 'user': tweet.user, 'text': tweet.text}
        return JsonResponse(tweet_object)
    except Exception:
        # import ipdb; ipdb.set_trace()
        return HttpResponse('ERROR')
