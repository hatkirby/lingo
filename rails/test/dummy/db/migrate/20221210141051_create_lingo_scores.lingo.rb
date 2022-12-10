# This migration comes from lingo (originally 20221210011146)
class CreateLingoScores < ActiveRecord::Migration[7.0]
  def change
    create_table :lingo_scores do |t|
      t.integer :user_id
      t.string :username
      t.string :avatar_url
      t.integer :score

      t.timestamps
    end
  end
end
