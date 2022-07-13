clear
clear npatk;

chsh = Setting(2);
chsh.Parties(1).AddMeasurement(2);
chsh.Parties(1).AddMeasurement(2);
chsh.Parties(2).AddMeasurement(2);
chsh.Parties(2).AddMeasurement(2);

matrix = chsh.MakeMomentMatrix(1);

disp(struct2table(matrix.SymbolTable));
disp(matrix.SymbolMatrix);
disp(matrix.SequenceMatrix);
 
p_table = matrix.ProbabilityTable;
disp(struct2table(p_table))

one_mmt = npatk('probability_table', matrix, [[2, 2]; [1, 2]]);
disp(struct2table(one_mmt));