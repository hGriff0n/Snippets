
class SourceStats
    def count_directory(dir)
        if File.file?(dir)
            # If the file is one of our supported extensions
            if @exts.has_key?(ext = File.extname(dir))

                total, sloc = 0, 0
                File.new(dir).each_line do |line|
                    comment_regex = nil
                    case ext
                        when ".rb"
                            comment_regex = /^(?![ \s]*\r?\n|[ \s]*#).*\r?\n/
                        when ".cpp", ".c", ".h"
                            # comment_regex = /^(?![ \s]*\r?\n|[ \s]*}\r?\n|[ \s]*\/\/|[ \s]*\/\*|[ \s]*\*).*\r?\n/
                            comment_regex = /^(?![ \s]*\r?\n|[ \s]*\/\/|[ \s]*\/\*|[ \s]*\*).*\r?\n/
                        else next
                    end

                    sloc += (line =~ comment_regex) ? 1 : 0
                    total += 1
                end
                
                @exts[ext][:files] += 1
                @exts[ext][:total] += total
                @exts[ext][:sloc] += sloc
                # puts "#{dir} => #{total} - #{sloc}"
            end

        # Otherwise recursively run through the structure
        else
            Dir.foreach(dir) do |file|
                next if file == '.' || file == '..'
                count_directory("#{dir}/#{file}")
            end
        end
    end

    def initialize(files)
        @exts = {
            '.cpp' => { :total => 0, :sloc => 0, :files => 0},
            '.h' => { :total => 0, :sloc => 0, :files => 0},
            '.c' => { :total => 0, :sloc => 0, :files => 0},
            '.rb' => { :total => 0, :sloc => 0, :files => 0}
        }

        files.each do |file|
            count_directory(file)
        end
    end

    attr_reader :exts
end