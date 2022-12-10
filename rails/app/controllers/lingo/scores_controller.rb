module Lingo
  class ScoresController < ApplicationController
    def index
      @scores = Score.order(score: :desc)
    end

    def update
      if params[:secret_code] != Lingo.secret_code then
        head :unauthorized
      else
        score = Score.find_or_create_by(user_id: params[:user_id])
        score.username = params[:username]
        score.avatar_url = params[:avatar_url]
        score.score += 1
        score.save!

        render :blank
      end
    end
  end
end
