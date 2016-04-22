
# from django.shortcuts import render
from django.http import HttpResponse, JsonResponse
from .api import Twitter
from .models import Tweet


def index(request):
    try:
        twitter = Twitter()
        topics = ['love', '#love']
        languages = ['en']
        twitter.stream(topics, languages)
        tweets = Tweet.objects.all()
        tweet = tweets[len(tweets) - 1]
        tweet_object = {'name': tweet.name, 'user': tweet.user, 'text': tweet.text}
        return JsonResponse(tweet_object, safe=False)
    except Exception:
        # import ipdb; ipdb.set_trace()
        return HttpResponse('ERROR')
