//Copyright(c) 11th Oct 2015 Joose Rajamäki
//
//Permission is hereby granted, free of charge, to any person obtaining
//a copy of this software and associated documentation files(the
//"Software"), to deal in the Software without restriction, including
//without limitation the rights to use, copy, modify, merge, publish,
//distribute, sublicense, and / or sell copies of the Software, and to
//permit persons to whom the Software is furnished to do so, subject to
//the following conditions :
//
//The above copyright notice and this permission notice shall be included
//in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include <math.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

unsigned int split(const std::string &txt, std::vector<std::string> &strs, char ch)
{
	unsigned int pos = txt.find(ch);
	unsigned int initialPos = 0;
	strs.clear();

	// Decompose statement
	while (pos != std::string::npos) {
		strs.push_back(txt.substr(initialPos, pos - initialPos + 1));
		initialPos = pos + 1;

		pos = txt.find(ch, initialPos);
	}

	// Add the last one
	strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

	return strs.size();
}

class Table {

public:

	std::vector<std::vector<std::string>> places; //The elements of places are the rows. The elements of the rows are the person names.

	Table(unsigned seatsInARow, unsigned rows) {

		for (unsigned row = 0; row < rows; row++) {
			places.push_back(std::vector<std::string>());
			for (unsigned place = 0; place < seatsInARow; place++) {
				places[row].push_back("");
			}
		}

	}

	bool set(const std::string& person, const unsigned& place, const unsigned& row = 0) {

		if (places.size() > row) {
			if (places[row].size() > place) {
				places[row][place] = person;
				return true;
			}
		}

		return false;

	}

	std::string get(const unsigned& place, const unsigned& row = 0) {

		std::string person = "not_a_place";

		if (places.size() > row) {
			if (places[row].size() > place) {
				person = places[row][place];
			}
		}

		return person;

	}

};

enum ReadingOperations
{
	PEOPLE, AFFINITIES, TABLES
};

class Party {

public:

	std::vector<std::string> people;
	std::map<std::pair<std::string, std::string>, double> affinities;
	std::vector<Table> tables;
	double energy;

	Party() {
		energy = 0.0;
	}

	void computeEnergy(void) {
		energy = 0.0;
		for (Table table : tables) {
			//Round table
			if (table.places.size() == 1) {

				for (int i = 0; i < table.places[0].size(); i++) {
					std::string person1 = table.get(i);
					std::string person2 = table.get((i + 1) % table.places[0].size());
					energy -= getAffinity(person1, person2);
				}

			}
			else { //Table with multiple people

				for (int row = 0; row < table.places.size(); row++) {
					for (int i = 0; i < table.places[0].size() - 1; i++) {
						std::string person1 = table.get(i, row);
						std::string person2 = table.get(i + 1, row);
						std::string personOpposite;
						if (row == 0) {
							personOpposite = table.get(i, row + 1);
						}
						energy -= getAffinity(person1, person2);
						if (row == 0) {
							energy -= getAffinity(person1, personOpposite);
						}
					}

					//End of the table
					if (row == 0) {
						std::string person = table.get(table.places[0].size() - 1, row);
						std::string personOpposite = table.get(table.places[0].size() - 1, row + 1);
						energy -= getAffinity(person, personOpposite);
					}

				}

			}
		}
	}

	double deltaEnergyOfHypotheticalChange(const unsigned& table1, const unsigned& table2, const unsigned& row1, const unsigned& row2, const unsigned& seat1, const unsigned& seat2) {

		double energyFreed = 0.0;
		double energyStored = 0.0;

		std::string person1ToMove = tables[table1].get(seat1, row1);
		std::vector<std::string> neighbors1;
		std::string person2ToMove = tables[table2].get(seat2, row2);
		std::vector<std::string> neighbors2;

		if (tables[table1].places.size() == 1) {

			int left = seat1 - 1;
			if (left < 0) {
				left = tables[table1].places[0].size() - 1;
			}
			int right = (seat1 + 1) % tables[table1].places[0].size();

			neighbors1.push_back(tables[table1].get(left));
			neighbors1.push_back(tables[table1].get(right));

		}
		else {

			int left = seat1 - 1;
			int right = seat1 + 1;

			if (left >= 0) {
				neighbors1.push_back(tables[table1].get(left, row1));
			}
			if (right < tables[table1].places[0].size()) {
				neighbors1.push_back(tables[table1].get(right, row1));
			}
			neighbors1.push_back(tables[table1].get(seat1, (row1 + 1) % 2)); //The person opposite

		}

		if (tables[table2].places.size() == 1) {

			int left = seat2 - 1;
			if (left < 0) {
				left = tables[table2].places[0].size() - 1;
			}
			int right = (seat2 + 1) % tables[table2].places[0].size();

			neighbors2.push_back(tables[table2].get(left));
			neighbors2.push_back(tables[table2].get(right));

		}
		else {

			int left = seat2 - 1;
			int right = seat2 + 1;

			if (left >= 0) {
				neighbors2.push_back(tables[table2].get(left, row2));
			}
			if (right < tables[table2].places[0].size()) {
				neighbors2.push_back(tables[table2].get(right, row2));
			}
			neighbors2.push_back(tables[table2].get(seat2, (row2 + 1) % 2)); //The person opposite

		}

		bool switchPlaces = false;

		for (auto& person : neighbors1) {
			energyFreed += -getAffinity(person, person1ToMove);
		}

		for (auto& person : neighbors2) {
			energyFreed += -getAffinity(person, person2ToMove);
		}

		for (auto& person : neighbors1) {
			if (person.compare(person2ToMove) == 0) {
				switchPlaces = true;
			}
			energyStored += -getAffinity(person, person2ToMove);
		}

		for (auto& person : neighbors2) {
			if (person.compare(person1ToMove) == 0) {
				switchPlaces = true;
			}
			energyStored += -getAffinity(person, person1ToMove);
		}

		if (switchPlaces) {
			energyStored += -2.0*getAffinity(person2ToMove, person1ToMove);
		}

		return energyStored - energyFreed;


	}

	void seatInitial(void) {

		unsigned personNumber = 0;

		for (Table& table : tables) {

			for (std::vector<std::string>& row : table.places) {

				for (std::string& seat : row) {

					seat = people[personNumber];
					personNumber++;

					if (personNumber >= people.size()) {
						break;
					}
				}

				if (personNumber >= people.size()) {
					break;
				}
			}


			if (personNumber >= people.size()) {
				break;
			}
		}

	}

	void addPeople(void) {
		std::string person = "Begin";
		while (person != "quit") {


			std::cout << "Add new person. The persons name cannot contain white space characters. Type 'quit' to quit." << std::endl;
			std::cin >> person;

			if (person != "quit") {

				people.push_back(person);

				std::sort(people.begin(), people.end());

				for (int i = 0; i < people.size() - 1; i++) {
					if (people[i] == people[i + 1]) {
						int counter = 1;
						while (i + counter < people.size() && people[i] == people[i + counter]) {
							people[i + counter] = people[i] + std::to_string(counter);
							counter++;
						}
					}
				}
			}
		}
	}

	void addAffinities(void) {

		for (int i = 0; i < people.size(); i++) {
			for (int j = i + 1; j < people.size(); j++) {

				while (true) {

					if (affinities.count(std::make_pair(people[i], people[j])) == 0) {

						double affinity = -std::numeric_limits<double>::infinity();
						std::string input;
						std::cout << "How much do " + people[i] + " and " + people[j] + " like each other? Give a number between -1 and +1. -1 means dislike and +1 means like. Type 'r' if they are in a relatioship." << std::endl;
						std::cin >> input;
						std::istringstream ss(input);

						if (input == "r") {
							double coupleAffinity = 10;
							affinities.insert(std::pair<std::pair<std::string, std::string>, double>(std::make_pair(people[i], people[j]), coupleAffinity));
							break;
						}

						ss >> affinity;

						if (std::abs(affinity) <= 1) {
							affinities.insert(std::pair<std::pair<std::string, std::string>, double>(std::make_pair(people[i], people[j]), affinity));
							break;
						}
					}
					else {
						break;
					}
				}

			}
		}

	}

	double getAffinity(const std::string& person1, const std::string& person2) {
		if (person1 == person2) {
			return 0.0;
		}

		if (person1 == "") {
			return 0.0;
		}

		if (person2 == "") {
			return 0.0;
		}

		if (person1 < person2) {
			return affinities[std::make_pair(person1, person2)];
		}
		else {
			return affinities[std::make_pair(person2, person1)];
		}
	}

	void writeToFile(const std::string& filename) {
		std::ofstream myfile;
		myfile.open(filename);

		myfile << "PEOPLE" << std::endl;

		for (auto& person : people) {
			myfile << person << std::endl;
		}

		myfile << "AFFINITIES" << std::endl;

		for (int i = 0; i < people.size(); i++) {
			for (int j = i + 1; j < people.size(); j++) {
				myfile << people[i] << " " << people[j] << " " << getAffinity(people[i], people[j]) << std::endl;
			}
		}

		myfile << "TABLES" << std::endl;

		for (Table& table : tables) {
			myfile << table.places.size() << " " << table.places[0].size() << std::endl;
		}

		myfile.close();
	}

	void readFromFile(const std::string& filename) {

		people.clear();
		affinities.clear();
		tables.clear();

		std::string line;
		std::ifstream myfile(filename);

		ReadingOperations operation = ReadingOperations::PEOPLE;

		if (myfile.is_open())
		{
			while (getline(myfile, line))
			{
				if (line == "PEOPLE") {
					operation = PEOPLE;
					continue;
				}
				else if (line == "AFFINITIES") {
					operation = AFFINITIES;
					continue;
				}
				else if (line == "TABLES") {
					operation = TABLES;
					continue;
				}
				else {
					if (operation == PEOPLE) {
						people.push_back(line);
					}
					else if (operation == AFFINITIES) {
						std::istringstream ss(line);
						std::string person1;
						ss >> person1;
						std::string person2;
						ss >> person2;
						float affinity = 0.0;
						ss >> affinity;
						affinities.insert(std::pair<std::pair<std::string, std::string>, double>(std::make_pair(person1, person2), affinity));
					}
					else if (operation == TABLES) {
						std::istringstream ss(line);
						int rows = 0;
						ss >> rows;
						int seatsPerRow = 0;
						ss >> seatsPerRow;
						Table table(seatsPerRow, rows);
						tables.push_back(table);
					}
				}
			}
			myfile.close();
		}
		else std::cout << "Unable to open file";
	}

	void addTables(void) {

		if (people.size() == 0) {
			std::cout << "Add people to the party first.";
			return;
		}

		std::string readValue = "Begin";
		while (readValue != "q") {

			int seatsInParty = 0;
			//Count the seats in the party. For greater efficiency this could be done just once.
			for (const Table& table : tables) {
				for (const std::vector<std::string>& row : table.places) {
					for (const std::string seat : row) {
						seatsInParty++;
					}
				}
			}

			if (seatsInParty < people.size()) {
				std::cout << "Not enough places for everyone. Add tables." << std::endl;
			}

			std::cout << "Add round table with 'r', add straight table with 's' or quit with 'q'." << std::endl;
			std::cin >> readValue;

			if (readValue != "q") {

				if (readValue == "r") {
					unsigned places = 0;
					std::cout << "Enter the number of places in this round table." << std::endl;
					std::cin >> places;

					Table table(places, 1);
					tables.push_back(table);
				}

				if (readValue == "s") {
					unsigned places = 0;
					std::cout << "Enter the number of places in a row for the straight table." << std::endl;
					std::cin >> places;

					Table table(places, 2);
					tables.push_back(table);
				}

			}
		}

	}

	void attemptChange(const double& temperature) {

		unsigned table1 = rand() % tables.size();
		unsigned row1 = rand() % tables[table1].places.size();
		unsigned seat1 = rand() % tables[table1].places[0].size();

		unsigned table2 = rand() % tables.size();
		unsigned row2 = rand() % tables[table2].places.size();
		unsigned seat2 = rand() % tables[table2].places[0].size();

		double energyChange = deltaEnergyOfHypotheticalChange(table1, table2, row1, row2, seat1, seat2);

		bool changePlace = false;
		if (energyChange < 0) {
			changePlace = true;
		}
		else {
			double ayn = (double)rand() / (double)RAND_MAX;
			if (ayn <= std::exp(-energyChange / temperature)) {
				changePlace = true;
			}
		}

		if (changePlace) {
			std::string person1 = tables[table1].get(seat1, row1);
			std::string person2 = tables[table2].get(seat2, row2);

			tables[table1].set(person2, seat1, row1);
			tables[table2].set(person1, seat2, row2);
			energy += energyChange;
		}

	}

	void timeStep(const double& temperature, const unsigned& numberOfSeatsInTheParty = 0) {

		unsigned seatsInParty = 0;

		if (numberOfSeatsInTheParty > 0) {
			seatsInParty = numberOfSeatsInTheParty;
		}
		else {
			//Count the seats in the party. For greater efficiency this could be done just once.
			for (const Table& table : tables) {
				for (const std::vector<std::string>& row : table.places) {
					for (const std::string seat : row) {
						seatsInParty++;
					}
				}
			}
		}

		for (unsigned i = 0; i < seatsInParty; i++) {
			attemptChange(temperature);
		}

	}

	void computePlaces(void) {

		seatInitial();

		double temperature = 5000;
		double annealing = 0.99;
		computeEnergy();

		double treshold = 1e-10;
		while (temperature > treshold) {

			timeStep(temperature);

			temperature *= annealing;

			double debugEnergy = energy;
			computeEnergy();
			energy = debugEnergy;

			std::cout << "Ready when this is zero: " << temperature << std::endl;

		}

	}

	void printResult(void) {

		for (const Table& table : tables) {

			if (table.places.size() == 1) {
				std::cout << "Round table" << std::endl;
				for (const auto& seat : table.places[0]) {
					std::cout << seat << std::endl;
				}
			}

			if (table.places.size() == 2) {
				std::cout << "Straight table" << std::endl;
				for (unsigned i = 0; i < table.places[0].size(); i++) {
					std::cout << table.places[0][i] << " " << table.places[1][i] << std::endl;
				}
			}

			std::cout << std::endl;

		}

	}

};


int main(void) {

	Party party;

	std::string tmp;
	std::cout << "Read the information from file? (yes/no)" << std::endl;
	std::cin >> tmp;

	if (tmp == "yes") {
		std::cout << "Name of the file to read from?" << std::endl;
		std::cin >> tmp;
		party.readFromFile(tmp);
	}
	else {
		std::cout << "Not reading info from a file." << std::endl;
	}

	std::cout << "Add people or tables? (yes/no)" << std::endl;
	std::cin >> tmp;

	if (tmp == "yes") {
		party.addPeople();
		party.addAffinities();
		party.addTables();
	}
	else {
		std::cout << "Not adding anything." << std::endl;
	}

	std::cout << "Save the current info to a file? (yes/no)" << std::endl;
	std::cin >> tmp;

	if (tmp == "yes") {
		std::cout << "Name of the file to write the data?" << std::endl;
		std::cin >> tmp;
		party.writeToFile(tmp);
	}
	else {
		std::cout << "Not saving info." << std::endl;
	}

	std::cout << "Computing the results next. Press enter to continue." << std::endl;
	std::getline(std::cin, tmp);
	std::getline(std::cin, tmp);

	party.computePlaces();

	std::cout << "You should seat the people in the following manner:" << std::endl;
	party.printResult();

	std::cout << "Press enter to exit.";
	std::getline(std::cin, tmp);

}
