require_relative "lib/lingo/version"

Gem::Specification.new do |spec|
  spec.name        = "lingo"
  spec.version     = Lingo::VERSION
  spec.authors     = ["Star Rauchenberger"]
  spec.email       = ["fefferburbia@gmail.com"]
  spec.homepage    = "https://github.com/hatkirby/lingo"
  spec.summary     = "Summary of Lingo."
  spec.description = "Description of Lingo."
    spec.license     = "MIT"

  # Prevent pushing this gem to RubyGems.org. To allow pushes either set the "allowed_push_host"
  # to allow pushing to a single host or delete this section to allow pushing to any host.

  spec.metadata["homepage_uri"] = spec.homepage
  spec.metadata["source_code_uri"] = "https://github.com/hatkirby/lingo"


  spec.files = Dir.chdir(File.expand_path(__dir__)) do
    Dir["{app,config,db,lib}/**/*", "MIT-LICENSE", "Rakefile", "README.md"]
  end

  spec.add_dependency "rails", ">= 7.0.3"
  spec.add_dependency "haml-rails", "~> 2.0"
  spec.add_dependency 'sassc-rails'
  spec.add_development_dependency 'webrick', '~> 1.7'
end
