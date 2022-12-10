module Lingo
  class ScoresController < ApplicationController
    skip_before_action :verify_authenticity_token, only: [:update]

    def index
      @scores = Score.order(score: :desc)
    end

    def update
      if params[:secret_code] != Lingo.secret_code then
        head :unauthorized
      else
        score = Score.find_or_create_by(user_id: params[:user_id]) do |score|
          score.score = 0
        end
        score.username = params[:username]
        score.avatar_url = CGI.unescape(params[:avatar_url])
        score.score += 1
        score.save!

        head :created
      end
    end
  end
end
