function read_data_from_folders(path,start_day, start_month, start_year, end_day, end_month, end_year,outfname)
    % cd(path)  % Change to the target data directory

    file_paths = {};  % Initialize a cell array to store valid file paths

    % Loop through each specified year, month, and day
    for year = start_year:end_year
        for month = start_month:end_month
            for day = start_day:end_day
                % Construct folder path in the format 'YYYY/MM/DD'
                folder_path = fullfile(path,sprintf('%04d', year), sprintf('%02d', month), sprintf('%02d', day));
                
                % If the folder exists, look for 'DATA.txt' inside it
                if exist(folder_path, 'dir')
                    file_path = fullfile(folder_path, 'DATA.txt');
                    
                    % If the file exists, add its path to the list
                    if exist(file_path, 'file')
                        file_paths{end+1} = file_path;
                    end
                end
            end
        end
    end

    % Create a map to track which lines (strings) have been seen already (for deduplication)
    lines_seen = containers.Map('KeyType', 'char', 'ValueType', 'logical');

    % Open the output file for writing
    fid_out = fopen(outfname, 'w');

    % Loop through each collected file
    for i = 1:length(file_paths)
        fid_in = fopen(file_paths{i}, 'r');  % Open the input file

        tline = fgetl(fid_in);  % Read the first line (typically the header)
        
        % Read and write lines that haven't been written yet
        while ischar(tline)
            if ~isKey(lines_seen, tline)  % Skip lines already written (including duplicate headers)
                fprintf(fid_out, '%s\n', tline);  % Write line to output file
                lines_seen(tline) = true;         % Mark line as written
            end
            tline = fgetl(fid_in);  % Read the next line
        end

        fclose(fid_in);  % Close the input file
    end

    fclose(fid_out);  % Close the output file after all writing is done
    
end
