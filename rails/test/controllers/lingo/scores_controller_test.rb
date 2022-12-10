require "test_helper"

module Lingo
  class ScoresControllerTest < ActionDispatch::IntegrationTest
    include Engine.routes.url_helpers

    test "should get index" do
      get scores_index_url
      assert_response :success
    end

    test "should get update" do
      get scores_update_url
      assert_response :success
    end
  end
end
