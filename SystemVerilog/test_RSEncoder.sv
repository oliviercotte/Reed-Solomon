
import reed_solomon::*;

// Testbench For the reed solomon encoder Design.

`timescale 1ns / 1ps

module test_RSEncoder ();
	
	parameter NB_BYTE_INPUT = 16;
	parameter NUMBER_OF_TEST_VECTOR = 100;

	// Define Clock and Reset Signals
	reg clock;
	reg reset;
	
	// Ports de communication
	logic[7:0] dataOutputPort[NB_BYTE_INPUT];
	logic encoderReadyPort;
	logic encoderValidPort;
	logic[7:0] encoderInputPort[NB_BYTE_INPUT];
	
	//---------------------------------------------------------
	// instantiate the Device Under Test (DUT)
	// using named instantiation
	RSEncoder RSEncoder_INST (
		.clockPort (clock),
		.resetPort (reset),
		.encoderInputPort (dataOutputPort),
		.encoderReadyPort (encoderReadyPort),
		.encoderValidPort (encoderValidPort),
		.encoderOutputPort (encoderInputPort)
	);

	// Clock Period set to 250MHz
	parameter clock_period = 40;
	always
	begin : Clock_Generator
		#(clock_period / 2) clock = ~clock;
	end

	initial
	begin
		clock <= 1'b0;
		
		// Initialisation des sorties
		dataOutputPort[0] = 0;
		dataOutputPort[1] = 0;
		dataOutputPort[2] = 0;
		dataOutputPort[3] = 0;
		dataOutputPort[4] = 0;
		dataOutputPort[5] = 0;
		dataOutputPort[6] = 0;
		dataOutputPort[7] = 0;
		dataOutputPort[8] = 0;
		dataOutputPort[9] = 0;
		dataOutputPort[10] = 0;
		dataOutputPort[11] = 0;
		dataOutputPort[12] = 0;
		dataOutputPort[13] = 0;
		dataOutputPort[14] = 0;
		dataOutputPort[15] = 0;
		
		// Met le reset sur un cycle
		$display($time, " << RESET >>");
		reset <= 1'b1;
		@(posedge clock);
		reset <= 1'b0;
		@(posedge clock);
		$display($time, " << Coming out of reset >>");
		
		test_bench();
	end
	
	task test_bench;
		logic[7:0] recd[BLOCK_LENGTH], data[MESSAGE_LENGTH];
		int counterIndex, error_index, no_encoder_errors, error_status[NB_BYTE_INPUT];
		
		$display ($time, "TB main loop start");
		for(int errorNumberIT = 0; errorNumberIT <= CORRECTION_CAPACITY+1; errorNumberIT=errorNumberIT+1) begin
			no_encoder_errors = 0;
			$display ($time, "Envoie de %d vecteurs contenant %d erreurs", NUMBER_OF_TEST_VECTOR, errorNumberIT);
			for(int j = 0; j < NUMBER_OF_TEST_VECTOR; j=j+1) begin
			
				///////////////////////////////////////
				// Comme dans la specification
				// executable, le(s) thread(s) doi(ven)t:
				// 
				// 1) Generer des paquets aleatoires

				// Creation d'un vecteur de test aleatoire 
				for (int i = 0; i < MESSAGE_LENGTH; i=i+1) begin
					data[i] = byte'{$random(j) % 256};
				end
				
				// 2) Les transferer a l'encodeur
				// Envoie des données du paquet (16 octets par cycle d'horloge)

				// Attente du "ready"
				@(encoderReadyPort);
				
				counterIndex = 0;
				while (counterIndex < MESSAGE_LENGTH) begin
					for (int i = 0; i < NB_BYTE_INPUT; i=i+1) begin
						dataOutputPort[i] = data[counterIndex];
						counterIndex = counterIndex + 1;
					end
					@(posedge clock);
				end
				
				// Il y a un cycle d'horloge entre le moment ou un module écrit une valeur sur son port de sortie et 
				// le moment ou l'autre module voit cette valeur apparaître sur son port d'entrée.
				@(encoderValidPort);
				
				// 3) Recuperer les paquets de sortie
				counterIndex = 0;
				while (counterIndex < BLOCK_LENGTH) begin
					for (int i = 0; i < NB_BYTE_INPUT; i=i+1) begin
						recd[counterIndex] = encoderInputPort[i];
						counterIndex = counterIndex + 1;
					end
					@(posedge clock);
				end
				
				// 4) Y injecter des erreurs
				for (int i = 0; i < BLOCK_LENGTH; ++i) begin
					error_status[i] = 0;
				end

				// Sélection aléatoires de i erreurs : 
				for (int k = 0; k < errorNumberIT; k=k+1) begin
					error_index = int'{$random(0) % 256};
					if (error_status[error_index] == 0) begin
						// Injection d'une erreur a l'indice choisi
						recd[error_index] = (recd[error_index] ^ $random(0)) % 256;
						error_status[error_index] = 1;
					end
					else begin
						k=k-1;
					end
				end
				
				// 5) Décoder les paquets avec erreurs
				reed_solomon.decode(recd);
				
				// 6) Verifier que les donnees peuvent etre recuperees.
				for (int k = 0; k < MESSAGE_LENGTH; k=k+1) begin
					if (data[k] != recd[k]) begin
						// Les donnees ont ete mal encodees
						no_encoder_errors++;
						break;
					end
				end
				
				if (no_encoder_errors == 0) begin
					$display ($time, " OK\n");
				end
				else begin
					$display ($time, "%d erreurs sur %d vecteurs.\n", no_encoder_errors, NUMBER_OF_TEST_VECTOR);
				end
			end
		end
	endtask

endmodule
