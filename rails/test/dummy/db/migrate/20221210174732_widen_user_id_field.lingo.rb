# This migration comes from lingo (originally 20221210174554)
class WidenUserIdField < ActiveRecord::Migration[7.0]
  def change
    change_column :lingo_scores, :user_id, :integer, limit: 8
  end
end
