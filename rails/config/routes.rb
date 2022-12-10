Lingo::Engine.routes.draw do
  root to: "scores#index"
  post "/update", to: "scores#update"
end
