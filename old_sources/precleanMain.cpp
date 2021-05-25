#include "Header.h"
#include <filesystem>
#include <span>
#define t // for time

//class layer {
//private:
//	v_real W;
//	bool reconstructed = false;
//public:
//	v_inte X_PCs; // start of each pulse
//	v_real Envelope;
//	v_real Waveform;
//	pint n;
//	pint fps;
//
//
//	layer(v_inte X, v_real E, v_real W, pint n_, pint fps_) {
//		X_PCs = X;
//		Envelope = E;
//		Waveform = W;
//		n = n_;
//		fps = fps_;
//	}
//	pint size() {
//		return X_PCs.size() + Envelope.size() + Waveform.size();
//	}
//	v_real reconstruct() {
//		if (reconstructed) {
//			return W;
//		}
//		boost::math::interpolators::cardinal_cubic_b_spline<real> spline(Waveform.begin(), Waveform.end(), 0.0, 1.0 / (real)Waveform.size());
//		v_real W_reconstructed(n, 0.0);
//
//		inte x0{ X_PCs[0] };
//		inte x1{ X_PCs[1] };
//		real step{ 1.0 / real(x1 - x0) };
//		for (pint i = 1; i < X_PCs.size() - 1; i++) {
//			x0 = X_PCs[i];
//			x1 = X_PCs[i + 1];
//			step = 1.0 / real(x1 - x0);
//			for (inte j = x0; j < x1; j++) {
//				W_reconstructed[j] = spline((j - x0) * step) * Envelope[i];
//			}
//			if (x1 - x0 <= 3) {
//				std::cout << "Warning: Pseudo cycle with period < 4 between " << x0 << " and " << x1 << "\n";
//			}
//		}
//
//		inte X_PCs_start = X_PCs[1];
//		inte X_PCs_end = X_PCs.back();
//		pint itens{ 5 };
//		real avg_t{ 0.0 };
//		real avg_e{ 0.0 };
//		while (X_PCs[0] > 0) {
//			avg_t = 0.0;
//			avg_e = 0.0;
//			for (pint i = 0; i < itens; i++) {
//				avg_t += X_PCs[i + 1] - X_PCs[i];
//				avg_e += Envelope[i];
//			}
//			X_PCs.insert(X_PCs.begin(), X_PCs[0] - std::round(avg_t / itens));
//			Envelope.insert(Envelope.begin(), avg_e / itens);
//		}
//
//		while (X_PCs.back() < n) {
//			avg_t = 0.0;
//			avg_e = 0.0;
//			for (pint i = X_PCs.size() - itens - 1; i < X_PCs.size() - 1; i++) {
//				avg_t += X_PCs[i + 1] - X_PCs[i];
//				avg_e += Envelope[i];
//			}
//			X_PCs.push_back(X_PCs.back() + std::round(avg_t / itens));
//			Envelope.push_back(avg_e / itens);
//		}
//
//		// Filling from 0 to Xp[1]
//		pint ctr = 0;
//		x0 = X_PCs[ctr];
//		x1 = X_PCs[ctr + 1];
//		while (x1 <= X_PCs_start) {
//			if (x1 < 0) {
//				continue;
//			}
//			else {
//				step = 1.0 / real(x1 - x0);
//				for (inte j = x0; j < x1; j++) {
//					if (j >= 0) {
//						W_reconstructed[j] = spline((j - x0) * step) * Envelope[ctr];
//					}
//				}
//			}
//			ctr++;
//			x0 = X_PCs[ctr];
//			x1 = X_PCs[ctr + 1];
//		}
//
//		ctr = X_PCs.size() - 1;
//		x0 = X_PCs[ctr - 1];
//		x1 = X_PCs[ctr];
//		while (x0 >= X_PCs_end) {
//			if (x0 >= n) {
//				continue;
//			}
//			else {
//				step = 1.0 / real(x1 - x0);
//				for (inte j = x0; j < x1; j++) {
//					if (j < n) {
//						W_reconstructed[j] = spline((j - x0) * step) * Envelope.back();
//					}
//				}
//			}
//			ctr--;
//			x0 = X_PCs[ctr - 1];
//			x1 = X_PCs[ctr];
//		}
//		reconstructed = true;
//		W = W_reconstructed;
//		return W_reconstructed;
//	}
//};

std::tuple<v_inte, v_real, v_real> compress(const v_real& W, inte max_seconds = 100, pint max_iterations = std::numeric_limits<pint>::max(), inte mult = 3) {
	#ifdef v
		std::cout << "inside compress\n";
	#endif
	v_pint posX, negX;
	get_pulses(W, posX, negX);

	if (posX.empty() || negX.empty()) {
		std::cout << "Unable to get pulses\n";
		throw "Unable to get pulses";
	}

	auto posF = get_frontier(W, posX);
	auto negF = get_frontier(W, negX);
	//write_vector(posF, name + "_pos.csv");
	//write_vector(negF, name + "_neg.csv");

	refine_frontier_iter(posF, W);
	refine_frontier_iter(negF, W);
	//write_vector(posF, name + "_pos_refined.csv");
	//write_vector(negF, name + "_neg_refined.csv");

	v_inte best_Xpcs = get_Xpcs(posF, negF);
	//write_vector(best_Xpcs, "Xpcs.csv");

	v_real best_avg;
	mode_abdm ma = average_pc_waveform(best_avg, best_Xpcs, W);
	//write_vector(best_avg, "avgpcw.csv");

	v_real best_envelope;
	auto [W_reconstructed, Envelope] = reconstruct(best_Xpcs, best_avg, W);
	best_envelope = Envelope;
	#ifdef v
		std::cout << "inside compress-1\n";
	#endif
	real lower_error = error(W_reconstructed, W);// average_pc_metric(best_avg, best_Xpcs, W);
	inte min_size = std::max(inte(10), inte(ma.mode) - inte(mult * ma.abdm));
	inte max_size = ma.mode + inte(mult * ma.abdm);
	#ifdef v
		std::cout << "inside compress-2\n";
	#endif

	// get sinusoid waveform
	auto fpa = rfft(W);
	#ifdef v
		std::cout << "inside compress-3\n";
		std::cout << "f=" << fpa[0] << " p=" << fpa[1] << " a=" << fpa[2] << "\n";
	#endif

	if (fpa[0] > 0) {
		pint T = pint(real(W.size()) / fpa[0]);
		v_real sinusoid(T, 0.0);
		for (pint i = 0; i < T; i++) {
			sinusoid[i] = fpa[2] * cos(fpa[1] + 2 * PI * fpa[0] * real(i) / W.size());
		}

		#ifdef v
			std::cout << "inside compress-4\n";
			write_vector(W, "W.csv");
			write_vector(sinusoid, "sinusoid.csv");
		#endif

		v_inte Xpcs_sinusoid = refine_Xpcs(W, sinusoid, T / 2, T * 2);
		auto [W_reconstructed_sin, Envelope_sin] = reconstruct(Xpcs_sinusoid, sinusoid, W);

		auto sinusoid_error = error(W_reconstructed_sin, W);
		std::cout << "Sinusoid error= " << sinusoid_error << ", Envelope error=" << lower_error << "\n";
		if (sinusoid_error < lower_error) {
			std::cout << "Using sinusoid as initial waveform:\n";
			std::cout << "f=" << fpa[0] << " p=" << fpa[1] << " a=" << fpa[2] << "\n";
			best_avg = sinusoid;
			min_size = T / 2;
			max_size = 2 * T;
		}
	}

	std::cout
		<< "\nInitial Info:\n"
		<< "Number of samples in the original signal:" << W.size() << "\n"
		<< "Appr. compressed size:" << best_Xpcs.size() + best_avg.size() << "\n"
		<< "Xpcs size:            " << best_Xpcs.size() << "\n"
		<< "Waveform length mode: " << ma.mode << "\n"
		<< "Waveform length abdm: " << ma.abdm << "\n"
		<< "Min waveform length:  " << min_size << "\n"
		<< "Max waveform length:  " << max_size << "\n"
		<< "Initial average error:" << lower_error << "\n";

	real avdv;
	v_inte Xpcs;
	v_real avg(best_avg);

	const std::chrono::time_point< std::chrono::steady_clock> start = std::chrono::high_resolution_clock::now();
	real duration = 0.0;
	real best_duration = 0.0;
	pint ctr = 0;
	pint best_iteration = 0;
	while (duration <= max_seconds * 1000 && ctr < max_iterations) {
		Xpcs = refine_Xpcs(W, avg, min_size, max_size);
		//std::cout << " refine_Xpcs";
		ma = average_pc_waveform(avg, Xpcs, W);
		//std::cout << " average_pc_waveform";
		inte min_size = std::max(inte(10), inte(ma.mode) - inte(mult * ma.abdm));
		inte max_size = ma.mode + inte(mult * ma.abdm);

		auto [W_reconstructed, Envelope] = reconstruct(Xpcs, avg, W);
		avdv = error(W_reconstructed, W);

		duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
		if (avdv < lower_error) {
			lower_error = avdv;
			best_Xpcs = Xpcs;
			best_envelope = Envelope;
			best_avg = avg;
			best_duration = duration;
			best_iteration = ctr + 1;

			//pint cn = 2 * (best_Xpcs.size() + best_avg.size()) + 1;
			std::cout
				<< "\n"
				<< "Iteration " << best_iteration << "\n"
				<< "time=" << best_duration << " ms\n"
				//<< "Number of samples in the original signal:" << W.size() << "\n"
				//<< "Appr. compressed size:" << cn << " (" <<  float(cn) / float(W.size()) << ")\n"
				<< "Waveform length mode: " << ma.mode << "\n"
				<< "Waveform length abdm: " << ma.abdm << "\n"
				<< "Min waveform length:  " << min_size << "\n"
				<< "Max waveform length:  " << max_size << "\n"
				<< "Average error:        " << lower_error << "\n";
		}
		//else {
		//	std::cout << "i=" << ctr << " t=" << duration << " ms e=" << avdv << " m=" << min_size << " M=" << max_size << " | ";
		//}
		ctr++;
	}
	std::cout << "\nCompressed in " << best_duration << " milliseconds, " << best_iteration << " iterations\n\n";
	//C.X_PCs, C.Waveform, C.Envelope
	return { best_Xpcs, best_avg, best_envelope };
}

//void compress_fd(const v_real& W) {
//
//	// get sinusoid waveform
//	auto fpa = rfft(W);
//
//	if (fpa[0] <= 0) {
//		std::cout << "error"; // TODO
//	}
//	pint T = pint(round(.8*(float)W.size()) / fpa[0]);
//	std::cout << T << "\n";
//	pint x0{ 0 };
//	pint adjust{ 0 };
//	v_real P;
//	v_real A;
//	real best_p{ 0.0 };
//	real best_a{ 0.0 };
//	pint best_T{ 0 };
//
//	while (x0 + T < W.size()) {
//		best_a = 0.0;
//		fpa = rfft(W.begin() + x0, W.begin() + x0 + T);
//		while (fpa[0] < 2 && x0 + T < W.size()) {
//			fpa = rfft(W.begin() + x0, W.begin() + x0 + T);
//			if (fpa[2] > best_a) {
//				best_a = fpa[2];
//				best_T = T;
//				best_p = fpa[1];
//			}
//			T++;
//		}
//		T = pint(round(.8 * (float)W.size()) / fpa[0]);
//		x0 += best_T;
//		P.push_back(x0);
//	}
//	write_vector(P, "phases.csv");
//}

///////////////////////////////////////////////////////////////////////////////////////

pint get_mode(v_pint& T) {
	std::sort(T.begin(), T.end());
	pint curr_value{ T[0] };
	pint curr_count = 0;
	pint max_count = 0;
	pint mode = 0;

	for (auto period: T){
		if (period == curr_value) {
			curr_count++;
		} else {
			if (curr_count > max_count) {
				max_count = curr_count;
				mode = curr_value;
			}
			curr_value = period;
			curr_count = 1;
		}
	}
	if (curr_count > max_count) {
		max_count = curr_count;
		mode = curr_value;
	}
	return mode;
}

v_real reconstruct_fd(const v_pint& Xp, const v_real& waveform, const v_real& E, pint n) {
	boost::math::interpolators::cardinal_cubic_b_spline<real> spline(waveform.begin(), waveform.end(), 0.0, 1.0 / (real)waveform.size());
	v_real W(n, 0.0);
	pint x0{ 0 };
	pint x1{ 0 };
	//real e;
	real step;
	for (pint i = 0; i < Xp.size() - 1; i++) {
		x0 = Xp[i];
		x1 = Xp[i + 1];
		step = 1.0 / real(x1 - x0);
		for (inte j = x0; j < x1; j++) {
			W[j] = spline((j - x0) * step) * E[i];
		}
		if (x1 - x0 <= 3) {
			std::cout << "Warning: Pseudo cycle with period < 4 between " << x0 << " and " << x1 << "\n";
		}
	}
	return W;
}

std::tuple<v_real, v_real> average_waveform_and_envelope(const v_pint& Xp, const v_real& W) {

	v_pint T(Xp.size() - 1);
	for (pint i = 0; i < Xp.size() - 1; ++i) {
		T[i] = Xp[i + 1] - Xp[i];
	}

	mode_abdm modeabdm = get_mode_and_abdm(T);
	pint mode{ modeabdm.mode };

	inte x0{ 0 };
	inte x1{ 0 };
	real step{ 1.0 / real(mode) };
	v_real waveform(mode);
	v_real E;
	std::fill(waveform.begin(), waveform.end(), 0.0);
	real amp;
	//#pragma loop(hint_parallel(4))
	for (pint i = 0; i < Xp.size() - 1; i++) {
		x0 = Xp[i];
		x1 = Xp[i + 1];
		amp = std::abs(*std::max_element(W.begin() + x0, W.begin() + x1, abs_compare));
		E.push_back(amp);
		if (x1 - x0 > 5) { // TODO
			boost::math::interpolators::cardinal_cubic_b_spline<real> spline(W.begin() + x0, W.begin() + x1, 0, 1.0 / real(x1 - x0));
			for (pint j = 0; j < mode; j++) {
				waveform[j] += spline(j * step);// *amp* amp;
			}
		}
	}
	amp = std::abs(*std::max_element(waveform.begin(), waveform.end(), abs_compare));
	for (pint i = 0; i < mode; i++) {
		waveform[i] = waveform[i] / amp;
	}


	return {waveform, E};
}

v_real center(v_real::const_iterator beg, v_real::const_iterator end) {
	v_real W(beg, end);
	float average{ 0.0 };
	for (size_t i = 0; i < W.size(); i++) {
		average += W[i];
	}
	average /= (float)W.size();
	for (size_t i = 0; i < W.size(); i++) {
		W[i] -= average;
	}
	return W;
}

//v_pint compress_fd(const v_real& W) {
//
//	auto fpa = rfft(W);
//	pint T = round((float)W.size() / (float)fpa[0]);
//	pint x0{ 0 };
//	v_real amplitudes;
//	float average_amplitude{0.0};
//	std::cout << T << "\n"; // HACK
//
//	while (x0 + T < W.size()) {
//		//std::cout <<"-"; // HACK
//		fpa = rfft(W.begin() + x0, W.begin() + x0 + T);
//		amplitudes.push_back(fpa[2]); // TODO
//		average_amplitude += fpa[2];
//		x0++;
//	}
//
//	std::cout << amplitudes.size() << "\n"; // HACK
//
//	average_amplitude /= (float)amplitudes.size();
//
//	for (size_t i = 0; i < amplitudes.size(); i++)	{
//		amplitudes[i] -= average_amplitude;
//	}
//
//	v_pint Xpcs;
//	fpa = rfft(amplitudes); // todo check for zero frequency
//	T = round((float)amplitudes.size() / (float)fpa[0]);
//
//	std::cout << T << "\n"; // HACK
//	v_real phases; // TODO
//
//	float best_p{ 0.0 };
//	pint best_x{ 0 };
//	bool inside_peak = false;
//	x0 = 0;
//	v_real segment;
//	while (x0 + T < amplitudes.size()) {
//		//std::cout << "+"; // HACK
//		segment = center(amplitudes.begin() + x0, amplitudes.begin() + x0 + T);
//		fpa = rfft(segment.begin(), segment.end());
//		phases.push_back(fpa[1]); // hack
//		if (std::pow(fpa[1], 3.0) > 25.0) {
//			if (fpa[1] > best_p) {
//				best_p = fpa[1];
//				best_x = x0;
//			}
//			inside_peak = true;
//		}
//		else {
//			if (inside_peak) {
//				Xpcs.push_back(best_x);
//				best_p = 0.0;
//				//x0 += T / 2; // hack
//			}
//			//p = 0.0;
//			inside_peak = false;
//		}
//		//P.push_back(p);
//		x0++;
//	}	
//	write_vector(amplitudes, "amplitudes.csv"); // TODO
//	write_vector(phases, "phases.csv"); // hack
//	return Xpcs;
//}


v_pint compress_fd(const v_real& W) {
	auto fpa = rfft(W);

	if (fpa[0] <= 0) {
		std::cout << "error"; // TODO
	}

	pint T = round((float)W.size() / (float)fpa[0]);
	v_real conv(6 * T, 0.0);
	real conv_val{ 0.0 };
	real best_val{ 0.0 };
	real best_offset{ 0.0 };
	for (size_t offset = T / 2; offset < 6 * T; offset++) {
		conv_val = 0.0;
		for (size_t i = 0; i < W.size() - offset; i++) {
			conv_val += W[i] * W[i + offset];
		}
		conv[offset] = conv_val;
		if (conv_val > best_val) {
			best_val = conv_val;
			best_offset = offset;
		}
	}

	T = best_offset;


	std::cout << T << "\n"; // HACK
	pint x0{ 0 };
	pint adjust{ 0 };
	v_pint Xpcs;
	v_real phases; // TODO
	v_real amplitudes; // TODO

	real p{ 0.0 };
	float best_p{ 0.0 };
	pint best_x{ 0 };
	bool inside_peak = false;
	while (x0 + T < W.size()) {
		//std::cout << x0 << "\n"; // HACK
		fpa = rfft(W.begin() + x0, W.begin() + x0 + T);

		//adjust = 1;
		//while (fpa[0] > 1) {
		//	fpa = rfft(W.begin() + x0, W.begin() + x0 + T - adjust);
		//	adjust++;
		//}
		//adjust = 1;
		//while (fpa[0] <= 0 && x0 + T + adjust < W.size()) {
		//	fpa = rfft(W.begin() + x0, W.begin() + x0 + T + adjust);
		//	adjust++;
		//}

		phases.push_back(fpa[1]); // TODO
		amplitudes.push_back(fpa[2]); // TODO
		if (std::pow(fpa[1], 3.0) > 25.0) {
			if (fpa[1] > best_p){
				best_p = fpa[1];
				best_x = x0;
			}
			inside_peak = true;
		} else {
			if (inside_peak) {
				Xpcs.push_back(best_x);
				best_p = 0.0;
				x0 = best_x + round(0.8 * (float)T); // hack
			}
			//p = 0.0;
			inside_peak = false;
		}
		//P.push_back(p);
		x0++;
	}



	std::cout << "best offset: " << best_offset << "\n";

	write_vector(conv, "conv.csv"); // TODO
	write_vector(phases, "phases.csv"); // TODO
	write_vector(amplitudes, "amplitudes.csv"); // TODO
	return Xpcs;
}

//void adjust_xpcs(v_pint& Xpcs, const v_real& W) {
//	v_pint L(Xpcs.size() - 1);
//	v_pint R(Xpcs.size() - 1);
//	//pint zero;
//	real distance;
//	pint best_x{ 0 };
//	real best_distance;
//	for (size_t i = 0; i < Xpcs.size() -1; i++) {
//		distance = 0.0;
//		best_distance = 2.0;
//		for (size_t x = Xpcs[i]; x < Xpcs[i+1]-1; x++) {
//			distance = abs(W[x]) + abs(W[x + 1]);
//			if (distance < best_distance) {
//				best_distance = distance;
//				best_x = x;
//			}
//		}
//		L[i] = best_x - Xpcs[i];
//		R[i] = Xpcs[i + 1] - best_x;
//	}
//	pint mode_L = get_mode(L);
//	pint mode_R = get_mode(R);
//	if (mode_L <= Xpcs[0]) {
//		for (size_t i = 0; i < Xpcs.size() - 1; i++){
//			Xpcs[i] -= mode_L;
//		}
//		std::cout << "shifted left\n";
//	} 
//	else if (mode_R + Xpcs.back() < W.size()) {
//		for (size_t i = 0; i < Xpcs.size() - 1; i++) {
//			Xpcs[i] += mode_R;
//		}
//		std::cout << "shifted right\n";
//	}
//	else if (mode_L < mode_R) {
//		Xpcs.erase(Xpcs.begin());
//		for (size_t i = 0; i < Xpcs.size() - 1; i++) {
//			Xpcs[i] -= mode_L;
//		}
//		std::cout << "removed first and shifted left\n";
//	}
//	else {
//		Xpcs.pop_back();
//		for (size_t i = 0; i < Xpcs.size() - 1; i++) {
//			Xpcs[i] += mode_R;
//		}
//		std::cout << "removed last and shifted right\n";
//	}
//}

template <typename T> int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

void adjust_xpcs(v_pint& Xpcs, const v_real& W) {
	v_pint L;
	v_pint R;
	//pint zero;
	//real distance;
	pint x0{ 0 };
	pint x1{ 0 };
	//real best_distance;
	for (size_t i = 0; i < Xpcs.size() - 1; i++) {
		x0 = Xpcs[i];
		x1 = Xpcs[i + 1];
		for (size_t x = x0; x < x1-1; x++) {
			if (W[x]==0.0) {
				L.push_back(x - x0);
				R.push_back(x1 - x);

			}
			else if (sign(W[x]) != sign(W[x + 1])){
				L.push_back(x - x0);
				R.push_back(x1 - x);
			}
		}		
	}
	std::cout << L.size() << " <><>" << R.size() << "\n";
	pint mode_L = get_mode(L);
	pint mode_R = get_mode(R);
	if (mode_L <= Xpcs[0]) {
		for (size_t i = 0; i < Xpcs.size() - 1; i++) {
			Xpcs[i] -= mode_L;
		}
		std::cout << "shifted left\n";
	}
	else if (mode_R + Xpcs.back() < W.size()) {
		for (size_t i = 0; i < Xpcs.size() - 1; i++) {
			Xpcs[i] += mode_R;
		}
		std::cout << "shifted right\n";
	}
	else if (mode_L < mode_R) {
		Xpcs.erase(Xpcs.begin());
		for (size_t i = 0; i < Xpcs.size() - 1; i++) {
			Xpcs[i] -= mode_L;
		}
		std::cout << "removed first and shifted left\n";
	}
	else {
		Xpcs.pop_back();
		for (size_t i = 0; i < Xpcs.size() - 1; i++) {
			Xpcs[i] += mode_R;
		}
		std::cout << "removed last and shifted right\n";
	}
}

void write_bin(std::string path, pint orig_n, pint fps, const v_pint& beg_of_pseudo_cycles, const v_real& waveform, const v_real& amp_of_pseudo_cycles) {
	std::cout << "n=" << orig_n << "\n";

	std::ofstream data_file;      // pay attention here! ofstream

	pint max_t = 0;
	v_pint T_pint(beg_of_pseudo_cycles.size());
	for (size_t i = 1; i < beg_of_pseudo_cycles.size(); i++) {
		T_pint[i] = beg_of_pseudo_cycles[i] - beg_of_pseudo_cycles[i - 1];
		if (T_pint[i] > max_t) {
			max_t = T_pint[i];
		}
	}

	v_pint pint_data = { orig_n, fps, amp_of_pseudo_cycles.size(), waveform.size(), max_t }; // header
	data_file.open(path, std::ios::out | std::ios::binary | std::fstream::trunc);
	data_file.write((char*)&pint_data[0], pint_data.size() * sizeof(pint));
	data_file.close();

	std::vector<char> T(beg_of_pseudo_cycles.size());
	T[0] = round(126.0 * (real)beg_of_pseudo_cycles[0] / (real)max_t);
	for (size_t i = 1; i < beg_of_pseudo_cycles.size(); i++) {
		T[i] = round(126.0 * (real)T_pint[i] / (real)max_t);
	}

	data_file.open(path, std::ios::out | std::ios::binary | std::fstream::app);
	data_file.write((char*)&T[0], T.size() * sizeof(char));
	data_file.close();


	std::vector<char> wv(waveform.size());
	for (size_t i = 0; i < waveform.size(); i++) {
		wv[i] = waveform[i] * 126.0;
	}
	data_file.open(path, std::ios::out | std::ios::binary | std::fstream::app);
	data_file.write((char*)&wv[0], wv.size() * sizeof(char));
	data_file.close();

	std::vector<char> en(amp_of_pseudo_cycles.size());
	for (size_t i = 0; i < amp_of_pseudo_cycles.size(); i++) {
		en[i] = amp_of_pseudo_cycles[i] * 126.0;
	}
	data_file.open(path, std::ios::out | std::ios::binary | std::fstream::app);
	data_file.write((char*)&en[0], en.size() * sizeof(char));
	data_file.close();

	write_vector(beg_of_pseudo_cycles, "written_XPC.csv"); // TODO

}

Wav read_bin(std::string path) {
	std::ifstream  data_file;
	data_file.open(path, std::ios::in | std::ios::binary);

	pint* header = new pint[5];
	data_file.read(reinterpret_cast<char*>(&header[0]), 5 * sizeof(pint));

	std::cout << "Decompressing file with n=" << header[0] << " and fps=" << header[1] << "\n";

	char* beg_of_pseudo_cycles_buffer = new char[header[2] + 1];
	data_file.read((char*)&beg_of_pseudo_cycles_buffer[0], (header[2] + 1) * sizeof(char));

	v_pint beg_of_pseudo_cycles(header[2] + 1);
	beg_of_pseudo_cycles[0] = round(beg_of_pseudo_cycles_buffer[0] + (real)header[4] / 126.0);
	for (size_t i = 1; i < beg_of_pseudo_cycles.size(); i++) {
		beg_of_pseudo_cycles[i] = round((real)beg_of_pseudo_cycles[i - 1] + (real)header[4] * (real)beg_of_pseudo_cycles_buffer[i] / 126.0);
	}

	char* waveform_buffer = new char[header[3]];
	data_file.read((char*)&waveform_buffer[0], (header[3]) * sizeof(char));
	v_real waveform(header[3]);
	for (size_t i = 0; i < header[3]; i++) {
		waveform[i] = (real)waveform_buffer[i] / 126.0;
	}

	char* envelope_buffer = new char[header[2]];
	data_file.read((char*)&envelope_buffer[0], (header[2]) * sizeof(char));
	v_real envelope(header[2]);
	for (size_t i = 0; i < header[2]; i++) {
		envelope[i] = (real)envelope_buffer[i] / 126.0;
	}

	data_file.close();

	write_vector(beg_of_pseudo_cycles, "read_XPC.csv"); // TODO


	// header[0] = n, header[1] = fps
	//reconstruct_fd(beg_of_pseudo_cycles,waveform, envelope, header[0]);
	return Wav(reconstruct_fd(beg_of_pseudo_cycles, waveform, envelope, header[0]), header[1]);
}


///////////////////////////////////////////////////////////////////////////////////////


v_real get_residue(const v_real& W0, const v_real& W1) {
	v_real R(W0.size(), 0.0);
	for (pint i = 0; i < R.size(); i++)	{
		R[i] = W0[i] - W1[i];
	}
	return R;
}

inline bool ends_with(std::string const& value, std::string const& ending) {
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

//void write_bin(std::string path, pint orig_n, pint fps, const v_inte& beg_of_pseudo_cycles, const v_real& waveform, const v_real& amp_of_pseudo_cycles) {
//	std::cout << "n=" << orig_n << "\n";
//
//	std::ofstream data_file;      // pay attention here! ofstream
//
//	pint max_t = 0;
//	v_pint T_pint(beg_of_pseudo_cycles.size());
//	for (size_t i = 1; i < beg_of_pseudo_cycles.size(); i++) {
//		T_pint[i] = beg_of_pseudo_cycles[i] - beg_of_pseudo_cycles[i - 1];
//		if (T_pint[i] > max_t) {
//			max_t = T_pint[i];
//		}
//	}
//
//	v_pint pint_data = { orig_n, fps, amp_of_pseudo_cycles.size(), waveform.size(), max_t }; // header
//	data_file.open(path, std::ios::out | std::ios::binary | std::fstream::trunc);
//	data_file.write((char*) &pint_data[0], pint_data.size() * sizeof(pint));
//	data_file.close();
//	
//	std::vector<char> T(beg_of_pseudo_cycles.size());
//	T[0] = beg_of_pseudo_cycles[0];
//	for (size_t i = 1; i < beg_of_pseudo_cycles.size(); i++) {
//		T[i] = round(126.0 * (real)T_pint[i] / (real)max_t);
//	}
//
//	data_file.open(path, std::ios::out | std::ios::binary | std::fstream::app);
//	data_file.write((char*)&T[0], T.size() * sizeof(char));
//	data_file.close();
//
//
//	std::vector<char> wv(waveform.size());
//	for (size_t i = 0; i < waveform.size(); i++) {
//		wv[i] = waveform[i] * 126.0;
//	}
//	data_file.open(path, std::ios::out | std::ios::binary | std::fstream::app);
//	data_file.write((char*)&wv[0], wv.size() * sizeof(char));
//	data_file.close();
//
//	std::vector<char> en(amp_of_pseudo_cycles.size());
//	for (size_t i = 0; i < amp_of_pseudo_cycles.size(); i++) {
//		en[i] = amp_of_pseudo_cycles[i] * 126.0;
//	}
//	data_file.open(path, std::ios::out | std::ios::binary | std::fstream::app);
//	data_file.write((char*)&en[0], en.size() * sizeof(char));
//	data_file.close();
//}


v_real decompress(v_inte X_PCs, v_real Envelope, v_real Waveform, pint n) {

	boost::math::interpolators::cardinal_cubic_b_spline<real> spline(Waveform.begin(), Waveform.end(), 0.0, 1.0 / (real)Waveform.size());
	v_real W_reconstructed(n, 0.0);

	inte x0{ X_PCs[0] };
	inte x1{ X_PCs[1] };
	real step{ 1.0 / real(x1 - x0) };
	for (pint i = 1; i < X_PCs.size() - 1; i++) {
		x0 = X_PCs[i];
		x1 = X_PCs[i + 1];
		step = 1.0 / real(x1 - x0);
		for (inte j = x0; j < x1; j++) {
			W_reconstructed[j] = spline((j - x0) * step) * Envelope[i];
		}
		if (x1 - x0 <= 3) {
			std::cout << "Warning: Pseudo cycle with period < 4 between " << x0 << " and " << x1 << "\n";
		}
	}

	inte X_PCs_start = X_PCs[1];
	inte X_PCs_end = X_PCs.back();
	pint itens{ 5 };
	real avg_t{ 0.0 };
	real avg_e{ 0.0 };
	while (X_PCs[0] > 0) {
		avg_t = 0.0;
		avg_e = 0.0;
		for (pint i = 0; i < itens; i++) {
			avg_t += X_PCs[i + 1] - X_PCs[i];
			avg_e += Envelope[i];
		}
		X_PCs.insert(X_PCs.begin(), X_PCs[0] - std::round(avg_t / itens));
		Envelope.insert(Envelope.begin(), avg_e / itens);
	}

	while (X_PCs.back() < n) {
		avg_t = 0.0;
		avg_e = 0.0;
		for (pint i = X_PCs.size() - itens - 1; i < X_PCs.size() - 1; i++) {
			avg_t += X_PCs[i + 1] - X_PCs[i];
			avg_e += Envelope[i];
		}
		X_PCs.push_back(X_PCs.back() + std::round(avg_t / itens));
		Envelope.push_back(avg_e / itens);
	}

	// Filling from 0 to Xp[1]
	pint ctr = 0;
	x0 = X_PCs[ctr];
	x1 = X_PCs[ctr + 1];
	while (x1 <= X_PCs_start) {
		if (x1 < 0) {
			continue;
		}
		else {
			step = 1.0 / real(x1 - x0);
			for (inte j = x0; j < x1; j++) {
				if (j >= 0) {
					W_reconstructed[j] = spline((j - x0) * step) * Envelope[ctr];
				}
			}
		}
		ctr++;
		x0 = X_PCs[ctr];
		x1 = X_PCs[ctr + 1];
	}

	ctr = X_PCs.size() - 1;
	x0 = X_PCs[ctr - 1];
	x1 = X_PCs[ctr];
	while (x0 >= X_PCs_end) {
		if (x0 >= n) {
			continue;
		}
		else {
			step = 1.0 / real(x1 - x0);
			for (inte j = x0; j < x1; j++) {
				if (j < n) {
					W_reconstructed[j] = spline((j - x0) * step) * Envelope.back();
				}
			}
		}
		ctr--;
		x0 = X_PCs[ctr - 1];
		x1 = X_PCs[ctr];
	}
	return W_reconstructed;
}





int main(int argc, char** argv) {
	pint max_secs = 20;
	pint max_iters = std::numeric_limits<pint>::max();
	bool save_csv = false;
	std::vector<std::string> wav_paths;
	std::vector<std::string> cmp_paths;
	std::string append = "reconstructed";
	for (int i = 1; i < argc; ++i) { // parsing args
		if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
			std::cout << "Size of char: " << sizeof(char) << " byte\n";
			std::cout << "Size of short: " << sizeof(short) << " byte\n";
			std::cout << "Size of int: " << sizeof(int) << " bytes\n";
			std::cout << "Size of float: " << sizeof(float) << " bytes\n";
			std::cout << "Size of double: " << sizeof(double) << " bytes\n\n";
			std::cout
				<< " For more info about the author: carlos-tarjano.web.app\n"
				<< " Usage: \n"
				<< " [-t x] [-i y] -[a z] [-csv] [path/to/file1.wav]...[path/to/filen.wav]  [path/to/file1.cmp]...[path/to/filem.cmp]\n"
				<< " -t or --time: (default " << max_secs <<" s) maximum time in seconds allowed for each compression task\n"
				<< " -i or --iterations: maximum number of iterations allowed for each compression task\n"
				<< " -a or --append: (default \"" << append << "\") string to be appended to each reconstructed file name\n"
				<< " -csv: saves a csv file with the beginning of each pseudo-cycle\n"
				<< " If no path is given the root folder will be scanned for .wav and .cmp files, and those will be processed accordingly\n";
			return 0;
		}
		else if (std::strcmp(argv[i], "-t") == 0 || std::strcmp(argv[i], "--time") == 0) {
			max_secs = strtol(argv[i + 1], nullptr, 0);
			std::cout << "Maximum time allowed: "<< max_secs << " seconds\n";
			i++;
		}
		else if (std::strcmp(argv[i], "-i") == 0 || std::strcmp(argv[i], "--iterations") == 0) {
			max_iters = strtol(argv[i + 1], nullptr, 0);
			std::cout << "Maximum iterations allowed: " << max_iters << " \n";
			i++;
		}
		else if (std::strcmp(argv[i], "-a") == 0 || std::strcmp(argv[i], "--append") == 0) {
			append = argv[i + 1];
			std::cout << "Append \"" << append << "\" to the name of reconstructed files\n";
			i++;
		}
		else if (std::strcmp(argv[i], "-csv") == 0) {
			save_csv = true;
			std::cout << "pseudo-cycle info will be saved as csv\n";
		}
		else if (ends_with(argv[i], ".wav")) {
			wav_paths.push_back(argv[i]);
		}
		else if (ends_with(argv[i], ".cmp")) {
			cmp_paths.push_back(argv[i]);
		}
	}

	if (wav_paths.empty() && cmp_paths.empty()) { // no files found in args, searching root
		for (const auto& entry : std::filesystem::directory_iterator("./")) {
			std::string path = { entry.path().u8string() };
			if (ends_with(path, ".wav")) {
				wav_paths.push_back(path);
			}
			else if (ends_with(path, ".cmp")) {
				cmp_paths.push_back(path);
			}
		}
	}

	for (auto path : wav_paths) {
		std::cout << "\nCompressing " << path << std::endl;
		Wav WV = read_wav(path);

		v_pint Xpcs = compress_fd(WV.W);
		adjust_xpcs(Xpcs, WV.W);
		auto [Waveform, Envelope] = average_waveform_and_envelope(Xpcs, WV.W);

		write_vector(Waveform, "waveform.csv");

		path.replace(path.end() - 4, path.end(), ".cmp");

		write_bin(path, WV.W.size(), WV.fps, Xpcs, Waveform, Envelope);

		if (save_csv) {
			path.replace(path.end() - 4, path.end(), ".csv");
			write_vector(Xpcs, path);
		}

		//reconstruct_fd(Xpcs, Waveform, Envelope, WV.W.size());
		Wav R(reconstruct_fd(Xpcs, Waveform, Envelope, WV.W.size()), WV.fps);
		R.write(path.replace(path.end() - 4, path.end(), "_recc.wav"));

		//try {
		//	auto [X_PCs, Waveform, Envelope] = compress(WV.W, max_secs, max_iters);
		//	path.replace(path.end() - 4, path.end(), ".cmp");
		//	write_bin(path, WV.W.size(), WV.fps, X_PCs, Waveform, Envelope);

		//	if (save_csv) {
		//		path.replace(path.end() - 4, path.end(), ".csv");
		//		write_vector(X_PCs, path);
		//	}

		//	v_real W_reconstructed;
		//	std::tie(W_reconstructed, Envelope) = reconstruct(X_PCs, Waveform, WV.W);
		//	Wav Wa(W_reconstructed, WV.fps);
		//	Wa.write(path.replace(path.end() - 4, path.end(), "_c.wav"));

		//	Wav R(get_residue(WV.W, Wa.W), WV.fps);
		//	R.write(path.replace(path.end() - 6, path.end(), "_r.wav"));
		//}
		//catch (...) {
		//	continue;
		//}
	}

	for (auto path : cmp_paths) {
		std::cout << "\nDecompressing " << path << std::endl;
		#ifdef period
			auto time = Chronograph();
		#endif
		auto WW = read_bin(path);
		#ifdef period
			time.stop("Read & decompressed " + path + " in ");
			auto time3 = Chronograph();
		#endif
		path.replace(path.end() - 4, path.end(), "_" + append + ".wav");
		WW.write(path);
		#ifdef period
			time3.stop("Wrote " + path + " in ");
		#endif
	}
}

