require 'sqlite3'

db = SQLite3::Database.open ARGV[0]
form_rows = db.query "SELECT form FROM forms WHERE proper = 0"

wanderlust = {}
wanderlust["w"] = "1"
wanderlust["a"] = "2"
wanderlust["n"] = "3"
wanderlust["d"] = "4"
wanderlust["e"] = "5"
wanderlust["r"] = "6"
wanderlust["l"] = "7"
wanderlust["u"] = "8"
wanderlust["s"] = "9"
wanderlust["t"] = "0"

forms = form_rows.map {|row| row[0].downcase}
orange_forms = forms.reject {|form| /^[wanderlust]*$/.match(form).nil?}.select {|form| form.length >= 3}
words = orange_forms.map do |form|
    oranged = form.chars.map {|letter| wanderlust[letter]}.join
    [form, oranged, oranged.to_i]
  end
words.sort_by! {|word| word[2]}

puts words.length
puts words[10]
puts words.sample
puts ""

oranges_by_num = {}
puzzles = []
words.each do |vals|
  oranges_by_num.each do |num, form|
    opposite = (vals[2] - num.to_i).to_s
    if oranges_by_num.include? opposite
      puzzles << ["#{form} + #{oranges_by_num[opposite]}", vals[0]]
      puzzles << ["#{oranges_by_num[opposite]} + #{form}", vals[0]]
    end
    opposite = "0" + opposite
    if oranges_by_num.include? opposite
      puzzles << ["#{form} + #{oranges_by_num[opposite]}", vals[0]]
      puzzles << ["#{oranges_by_num[opposite]} + #{form}", vals[0]]
    end
  end
  oranges_by_num[vals[1]] = vals[0]
end

puts puzzles.length
puts puzzles.sample

File.open("wanderlust.txt", "w") do |f|
  puzzles.each do |puzzle|
    f.write("#{puzzle[0]}\n#{puzzle[1]}\n")
  end
end
