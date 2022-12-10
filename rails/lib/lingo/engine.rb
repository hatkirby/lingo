require 'haml'

module Lingo
  class Engine < ::Rails::Engine
    isolate_namespace Lingo

    initializer "pokeviewer.assets" do |app|
      app.config.assets.precompile += %w(lingo/header.png)
    end
  end
end
