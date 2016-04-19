
# from django.shortcuts import render
from django.http import HttpResponse
from .api import Twitter
from .models import Tweet


def index(request):
    try:
        twitter = Twitter()
        twitter.stream('amor')
        tweets = Tweet.objects.all()
        return HttpResponse(tweets[len(tweets) - 1].text)
    except Exception:
        import ipdb; ipdb.set_trace()
        # return HttpResponse('ERROR')
