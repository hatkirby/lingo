require 'haml'

module Lingo
  class Engine < ::Rails::Engine
    isolate_namespace Lingo
  end
end
