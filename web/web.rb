require 'rack/cache'
require 'yaml'

require 'rubygems'
require 'bundler/setup'
Bundler.require :default

use Rack::Cache

config = YAML.load(open("config.yml"))
db = Sequel.connect("sqlite://#{config["database"]}")

class LingoScore < Sequel::Model
end

get '/' do
  @scores = LingoScore.reverse_order(:score)

  haml :index
end

post '/update' do
  if params[:secret_code] != config["secret_code"] then
    403
  else
    if LingoScore.where(user_id: params[:user_id]).count == 0
      score = LingoScore.new(score: 0)
    else
      score = LingoScore.first(user_id: params[:user_id])
    end
    score.username = params[:username]
    score.avatar_url = CGI.unescape(params[:avatar_url])
    score.score += 1
    score.save

    201
  end
end

get '/style.css' do
  cache_control :public, :max_age => 36000

  scss :style
end
