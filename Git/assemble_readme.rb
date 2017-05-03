# Simple ruby script that constructs the readme file from various sources
# The primary purpose of this script is to automate the process of updating semantic versioning
# However, this can also be extended in the future to perform various tasks

require_relative 'utils'
require 'yaml'

# puts ARGV.inspect

yaml_file = "#{Dir.pwd}/.git/hooks/version.yaml"
ver = YAML.load_file(yaml_file)
File.open(Dir.pwd + "/README.md", "w") do |readme|
    # Semantic versioning (TODO: Determine what needs updating)
    case ARGV[1]
        when "1"
            ver['major'] += 1
        when "2"
            ver['minor'] += 1
        else
            ver['patch'] += 1
    end
    readme.puts "speroc ver #{ver['major']}.#{ver['minor']}.#{ver['patch']} - The reference compiler for the spero language"


    # Information about files in the project
        # NOTE: This isn't going to be accurate if source files are changed but not added to the commit
    stats = SourceStats.new(["incl", "src", "main.cpp", "test.rb"])
    num_lines = stats.exts.reduce(0) do |acc, elem|
        acc + elem[1][:sloc]
    end

    readme.puts "\nProject Info:"
    readme.puts "\n    size: #{num_lines} sloc"
    readme.puts "    #{stats.exts['.h'][:files]} .h files, #{stats.exts['.cpp'][:files]} .cpp files, #{stats.exts['.rb'][:files]} .rb files"


    # TODO: Other stuff that I can possibly add in 


    # Move over the custom readme information into the public readme
    readme.puts ""
    File.open(Dir.pwd + "/_readme.md", "r").each_line do |line|
        readme.puts line
    end
end if ARGV[0] == "0"

# TODO: Look at adding more stats to the SourceStats resolver class (in utils.rb)
File.open(yaml_file, 'w') {|f| f.write ver.to_yaml }