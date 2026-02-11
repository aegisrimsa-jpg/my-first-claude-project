// =============================================
// 小朋友計算機 - 升級版
// 功能：按鈕介面 / 智慧解題步驟 / 多語言重點整理 / 計算記錄與下載
// =============================================

// ===== 狀態 =====
let expression = "";
let currentLang = "zh";
let history = [];

// ===== DOM =====
const display = document.getElementById("display");
const stepsEl = document.getElementById("steps");
const tipsEl = document.getElementById("tips");
const historyListEl = document.getElementById("historyList");

// ===== 多語言文字 =====
const i18n = {
  zh: {
    title: "小朋友計算機",
    stepsTitle: "計算過程",
    tipsTitle: "重點整理",
    historyTitle: "計算記錄",
    stepsPlaceholder: "按下 = 後顯示解題步驟",
    tipsPlaceholder: "計算後會顯示學習重點",
    historyPlaceholder: "尚無計算記錄",
    errorMsg: "算式有誤，請重新輸入！",
    step: "第{n}步",
    takeNumber: "取數字 {num}",
    add: "加上",
    subtract: "減去",
    multiply: "乘以",
    divide: "除以",
    operateWith: "{op} {num}",
    openParen: "開始括號運算",
    closeParen: "結束括號運算，得到括號內結果",
    bracketFirst: "先算括號裡面的：{expr} = {val}",
    mulDivFirst: "先算乘除：{expr} = {val}",
    addSubNext: "再算加減：{expr} = {val}",
    finalResult: "最終結果 {expr} = {result}",
    tipOrder: "先乘除、後加減：遇到加減和乘除混在一起時，要先算乘除再算加減。",
    tipBracket: "括號最優先：有括號時，要先算括號裡面的內容。",
    tipAdd: "加法就是把兩個數合在一起，例如 3 + 2 就是把 3 和 2 合起來變成 5。",
    tipSub: "減法就是從一個數裡面拿走一些，例如 5 − 2 就是從 5 拿走 2 變成 3。",
    tipMul: "乘法是快速的加法，例如 3 × 4 等於把 3 加 4 次：3 + 3 + 3 + 3 = 12。",
    tipDiv: "除法是平均分配，例如 12 ÷ 3 就是把 12 平均分成 3 份，每份 4 個。",
    tipDecimal: "小數點讓我們可以表示不到 1 的部分，例如 0.5 就是一半。",
    tipDivByZero: "任何數除以 0 是沒有意義的，計算機會顯示錯誤。",
    downloadFileName: "計算記錄",
    resultLabel: "結果",
    timeLabel: "時間",
  },
  en: {
    title: "Kids Calculator",
    stepsTitle: "Steps",
    tipsTitle: "Key Points",
    historyTitle: "History",
    stepsPlaceholder: "Press = to see solving steps",
    tipsPlaceholder: "Key points shown after calculation",
    historyPlaceholder: "No history yet",
    errorMsg: "Invalid expression, try again!",
    step: "Step {n}",
    takeNumber: "Take number {num}",
    add: "Plus",
    subtract: "Minus",
    multiply: "Times",
    divide: "Divided by",
    operateWith: "{op} {num}",
    openParen: "Start parentheses",
    closeParen: "End parentheses, get inner result",
    bracketFirst: "Parentheses first: {expr} = {val}",
    mulDivFirst: "Multiply/divide first: {expr} = {val}",
    addSubNext: "Then add/subtract: {expr} = {val}",
    finalResult: "Final result: {expr} = {result}",
    tipOrder: "Multiply & divide before add & subtract: When mixed, always do × and ÷ first.",
    tipBracket: "Parentheses first: Always calculate what's inside ( ) before anything else.",
    tipAdd: "Addition combines numbers. 3 + 2 means putting 3 and 2 together to get 5.",
    tipSub: "Subtraction takes away. 5 − 2 means taking 2 from 5 to get 3.",
    tipMul: "Multiplication is fast addition. 3 × 4 means adding 3 four times: 3+3+3+3 = 12.",
    tipDiv: "Division is sharing equally. 12 ÷ 3 means splitting 12 into 3 groups of 4.",
    tipDecimal: "Decimals represent parts less than 1. For example, 0.5 is one half.",
    tipDivByZero: "Dividing by zero is undefined — the calculator will show an error.",
    downloadFileName: "calculation-history",
    resultLabel: "Result",
    timeLabel: "Time",
  },
  ja: {
    title: "キッズ電卓",
    stepsTitle: "計算の手順",
    tipsTitle: "ポイント整理",
    historyTitle: "計算履歴",
    stepsPlaceholder: "= を押すと解き方が表示されます",
    tipsPlaceholder: "計算後に学習ポイントを表示します",
    historyPlaceholder: "履歴はまだありません",
    errorMsg: "式が正しくありません。もう一度入力してください！",
    step: "ステップ{n}",
    takeNumber: "数字 {num} を取る",
    add: "足す",
    subtract: "引く",
    multiply: "かける",
    divide: "割る",
    operateWith: "{op} {num}",
    openParen: "カッコの計算を開始",
    closeParen: "カッコの計算を終了、カッコ内の結果を得る",
    bracketFirst: "カッコの中を先に計算：{expr} = {val}",
    mulDivFirst: "掛け算・割り算を先に：{expr} = {val}",
    addSubNext: "次に足し算・引き算：{expr} = {val}",
    finalResult: "最終結果 {expr} = {result}",
    tipOrder: "掛け算・割り算が先：足し算・引き算と混ざっている時は、掛け算・割り算を先に計算します。",
    tipBracket: "カッコが最優先：カッコがある時は、カッコの中を先に計算します。",
    tipAdd: "足し算は2つの数を合わせること。3 + 2 は 3 と 2 を合わせて 5 になります。",
    tipSub: "引き算は数を取り除くこと。5 − 2 は 5 から 2 を取って 3 になります。",
    tipMul: "掛け算は速い足し算。3 × 4 は 3 を 4 回足す：3+3+3+3 = 12。",
    tipDiv: "割り算は均等に分けること。12 ÷ 3 は 12 を 3 つに分けて、1つ 4 です。",
    tipDecimal: "小数点は 1 より小さい部分を表します。例えば 0.5 は半分です。",
    tipDivByZero: "0 で割ることはできません。電卓はエラーを表示します。",
    downloadFileName: "計算履歴",
    resultLabel: "結果",
    timeLabel: "時間",
  },
};

// 取得翻譯
function t(key) {
  return i18n[currentLang][key] || i18n["zh"][key] || key;
}

// 運算符對應翻譯 key
const opLangKeys = {
  "+": "add",
  "-": "subtract",
  "*": "multiply",
  "/": "divide",
};

// 運算符顯示符號
const opDisplay = { "+": "+", "-": "−", "*": "×", "/": "÷" };

// ===== 語言切換 =====
function setLang(lang) {
  currentLang = lang;
  // 更新按鈕狀態
  document.querySelectorAll(".lang-btn").forEach(function (btn) {
    btn.classList.toggle("active", btn.getAttribute("data-lang") === lang);
  });
  // 更新 UI 文字
  document.getElementById("mainTitle").textContent = t("title");
  document.getElementById("stepsTitle").textContent = t("stepsTitle");
  document.getElementById("tipsTitle").textContent = t("tipsTitle");
  document.getElementById("historyTitle").textContent = t("historyTitle");
  // 更新 placeholder
  var sp = document.getElementById("stepsPlaceholder");
  if (sp) sp.textContent = t("stepsPlaceholder");
  var tp = document.getElementById("tipsPlaceholder");
  if (tp) tp.textContent = t("tipsPlaceholder");
  var hp = document.getElementById("historyPlaceholder");
  if (hp) hp.textContent = t("historyPlaceholder");
  // 重新渲染歷史記錄
  renderHistory();
}

// ===== 輸入字元 =====
function inputChar(ch) {
  if (expression === "" && ch === "0") {
    expression = "0";
  } else {
    expression += ch;
  }
  updateDisplay();
}

// ===== 退格 =====
function backspace() {
  expression = expression.slice(0, -1);
  updateDisplay();
}

// ===== 清除 =====
function clearAll() {
  expression = "";
  display.textContent = "0";
  stepsEl.innerHTML = '<div class="step-placeholder" id="stepsPlaceholder">' + t("stepsPlaceholder") + "</div>";
  tipsEl.innerHTML = '<div class="tip-placeholder" id="tipsPlaceholder">' + t("tipsPlaceholder") + "</div>";
}

// ===== 更新顯示幕 =====
function updateDisplay() {
  var text = expression
    .replace(/\*/g, "×")
    .replace(/\//g, "÷")
    .replace(/-/g, "−");
  display.textContent = text || "0";
}

// ===== 計算 =====
function calculate() {
  if (expression === "") return;

  stepsEl.innerHTML = "";
  tipsEl.innerHTML = "";

  var result;
  try {
    if (!/^[\d+\-*/().  ]+$/.test(expression)) {
      throw new Error("invalid");
    }
    result = Function('"use strict"; return (' + expression + ")")();
    if (!isFinite(result)) {
      throw new Error("infinite");
    }
  } catch (e) {
    addStep(t("errorMsg"), "error");
    return;
  }

  // 智慧解題步驟
  generateSmartSteps(expression, result);

  // 多語言重點整理
  generateTips(expression);

  // 存入歷史
  addHistory(expression, result);

  // 將結果放回顯示幕
  expression = String(result);
  updateDisplay();
}

// ===== 智慧解題步驟 =====
function generateSmartSteps(expr, finalResult) {
  var tokens = tokenize(expr);
  var steps = [];
  var stepNum = 1;

  // 只有一個數字
  if (tokens.length === 1) {
    addStep(t("step").replace("{n}", stepNum) + "：" + t("takeNumber").replace("{num}", tokens[0]));
    stepNum++;
    addStep(
      t("step").replace("{n}", stepNum) + "：" + t("finalResult").replace("{expr}", formatExpression(expr)).replace("{result}", formatResult(finalResult)),
      "result"
    );
    return;
  }

  var hasBrackets = expression.includes("(");
  var hasMulDiv = expression.includes("*") || expression.includes("/");
  var hasAddSub = /[+\-]/.test(expression.replace(/^\-/, ""));

  // 如果有括號 → 先算括號
  if (hasBrackets) {
    var bracketExprs = extractBracketExpressions(expr);
    for (var i = 0; i < bracketExprs.length; i++) {
      var be = bracketExprs[i];
      var bracketResult;
      try {
        bracketResult = Function('"use strict"; return (' + be + ")")();
      } catch (e) {
        bracketResult = "?";
      }
      addStep(
        t("step").replace("{n}", stepNum) + "：" +
        t("bracketFirst").replace("{expr}", formatExpression(be)).replace("{val}", formatResult(bracketResult))
      );
      stepNum++;
    }
  }

  // 如果有乘除和加減混合 → 說明先乘除
  if (hasMulDiv && hasAddSub) {
    // 找出乘除子表達式
    var mulDivParts = findMulDivSubExpressions(tokens);
    for (var j = 0; j < mulDivParts.length; j++) {
      var md = mulDivParts[j];
      var mdResult;
      try {
        mdResult = Function('"use strict"; return (' + md.expr + ")")();
      } catch (e) {
        mdResult = "?";
      }
      addStep(
        t("step").replace("{n}", stepNum) + "：" +
        t("mulDivFirst").replace("{expr}", formatExpression(md.expr)).replace("{val}", formatResult(mdResult))
      );
      stepNum++;
    }

    // 再說明加減
    addStep(
      t("step").replace("{n}", stepNum) + "：" +
      t("addSubNext").replace("{expr}", formatExpression(expr)).replace("{val}", formatResult(finalResult))
    );
    stepNum++;
  } else if (!hasBrackets) {
    // 純加減或純乘除，逐步計算
    var running = parseFloat(tokens[0]);
    addStep(t("step").replace("{n}", stepNum) + "：" + t("takeNumber").replace("{num}", tokens[0]));
    stepNum++;

    for (var k = 1; k < tokens.length; k += 2) {
      if (k + 1 < tokens.length) {
        var op = tokens[k];
        var num = tokens[k + 1];
        var opName = t(opLangKeys[op]);
        var prev = running;

        switch (op) {
          case "+": running += parseFloat(num); break;
          case "-": running -= parseFloat(num); break;
          case "*": running *= parseFloat(num); break;
          case "/": running /= parseFloat(num); break;
        }

        addStep(
          t("step").replace("{n}", stepNum) + "：" +
          t("operateWith").replace("{op}", opName).replace("{num}", num) +
          "  →  " + formatResult(prev) + " " + opDisplay[op] + " " + num + " = " + formatResult(running)
        );
        stepNum++;
      }
    }
  }

  // 最終結果
  addStep(
    t("step").replace("{n}", stepNum) + "：" +
    t("finalResult").replace("{expr}", formatExpression(expr)).replace("{result}", formatResult(finalResult)),
    "result"
  );
}

// ===== 提取括號中的表達式 =====
function extractBracketExpressions(expr) {
  var results = [];
  var depth = 0;
  var start = -1;
  for (var i = 0; i < expr.length; i++) {
    if (expr[i] === "(") {
      if (depth === 0) start = i + 1;
      depth++;
    } else if (expr[i] === ")") {
      depth--;
      if (depth === 0 && start !== -1) {
        results.push(expr.substring(start, i));
        start = -1;
      }
    }
  }
  return results;
}

// ===== 找出乘除子表達式 =====
function findMulDivSubExpressions(tokens) {
  var results = [];
  var i = 0;
  while (i < tokens.length) {
    if (i + 2 < tokens.length && (tokens[i + 1] === "*" || tokens[i + 1] === "/")) {
      var parts = [tokens[i]];
      var j = i + 1;
      while (j < tokens.length && (tokens[j] === "*" || tokens[j] === "/")) {
        parts.push(tokens[j], tokens[j + 1]);
        j += 2;
      }
      if (parts.length >= 3) {
        results.push({ expr: parts.join(""), start: i, end: j - 1 });
      }
      i = j;
    } else {
      i++;
    }
  }
  return results;
}

// ===== 多語言重點整理 =====
function generateTips(expr) {
  var tips = [];
  var hasAdd = expr.includes("+");
  var hasSub = expr.includes("-");
  var hasMul = expr.includes("*");
  var hasDiv = expr.includes("/");
  var hasBrackets = expr.includes("(");
  var hasDecimal = expr.includes(".");
  var hasMixed = (hasMul || hasDiv) && (hasAdd || hasSub);

  // 運算順序提示
  if (hasMixed) {
    tips.push({ icon: "📐", text: t("tipOrder") });
  }
  if (hasBrackets) {
    tips.push({ icon: "🔢", text: t("tipBracket") });
  }

  // 運算說明
  if (hasAdd) tips.push({ icon: "➕", text: t("tipAdd") });
  if (hasSub) tips.push({ icon: "➖", text: t("tipSub") });
  if (hasMul) tips.push({ icon: "✖️", text: t("tipMul") });
  if (hasDiv) tips.push({ icon: "➗", text: t("tipDiv") });

  // 特殊提示
  if (hasDecimal) tips.push({ icon: "🔵", text: t("tipDecimal") });
  if (hasDiv && expr.match(/\/\s*0(?!\d)/)) {
    tips.push({ icon: "⚠️", text: t("tipDivByZero") });
  }

  if (tips.length === 0) return;

  for (var i = 0; i < tips.length; i++) {
    var div = document.createElement("div");
    div.className = "tip-item";
    div.innerHTML = '<span class="tip-icon">' + tips[i].icon + "</span>" + escapeHtml(tips[i].text);
    tipsEl.appendChild(div);
  }
}

// ===== 計算記錄 =====
function addHistory(expr, result) {
  var entry = {
    expr: expr,
    result: result,
    displayExpr: formatExpression(expr),
    displayResult: formatResult(result),
    time: new Date(),
  };
  history.unshift(entry);
  if (history.length > 50) history.pop();
  renderHistory();
}

function renderHistory() {
  if (history.length === 0) {
    historyListEl.innerHTML = '<div class="history-placeholder" id="historyPlaceholder">' + t("historyPlaceholder") + "</div>";
    return;
  }

  historyListEl.innerHTML = "";
  for (var i = 0; i < history.length; i++) {
    var entry = history[i];
    var div = document.createElement("div");
    div.className = "history-item";
    div.setAttribute("data-index", i);
    div.onclick = (function (e) {
      return function () {
        loadFromHistory(e);
      };
    })(entry);

    var exprDiv = document.createElement("div");
    exprDiv.className = "history-expr";
    exprDiv.textContent = entry.displayExpr;

    var resultDiv = document.createElement("div");
    resultDiv.className = "history-result";
    resultDiv.textContent = "= " + entry.displayResult;

    var timeDiv = document.createElement("div");
    timeDiv.className = "history-time";
    timeDiv.textContent = formatTime(entry.time);

    div.appendChild(exprDiv);
    div.appendChild(resultDiv);
    div.appendChild(timeDiv);
    historyListEl.appendChild(div);
  }
}

function loadFromHistory(entry) {
  expression = entry.expr;
  updateDisplay();
  // 重新計算以顯示步驟
  calculate();
}

function clearHistory() {
  history = [];
  renderHistory();
}

// ===== 下載歷史記錄 =====
function downloadHistory(format) {
  if (history.length === 0) return;

  var content = "";
  var filename = "";
  var mime = "";

  if (format === "csv") {
    content = t("timeLabel") + "," + "Expression" + "," + t("resultLabel") + "\n";
    for (var i = 0; i < history.length; i++) {
      var e = history[i];
      content += '"' + formatTime(e.time) + '","' + e.displayExpr + '","' + e.displayResult + '"\n';
    }
    filename = t("downloadFileName") + ".csv";
    mime = "text/csv;charset=utf-8";
  } else {
    content = "=== " + t("historyTitle") + " ===\n\n";
    for (var j = 0; j < history.length; j++) {
      var h = history[j];
      content += (j + 1) + ". " + h.displayExpr + " = " + h.displayResult + "\n";
      content += "   " + t("timeLabel") + ": " + formatTime(h.time) + "\n\n";
    }
    filename = t("downloadFileName") + ".txt";
    mime = "text/plain;charset=utf-8";
  }

  var blob = new Blob(["\uFEFF" + content], { type: mime });
  var url = URL.createObjectURL(blob);
  var a = document.createElement("a");
  a.href = url;
  a.download = filename;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}

// ===== 工具函式 =====

function tokenize(expr) {
  var result = [];
  var current = "";
  for (var i = 0; i < expr.length; i++) {
    var ch = expr[i];
    if ("0123456789.".includes(ch)) {
      current += ch;
    } else {
      if (current !== "") {
        result.push(current);
        current = "";
      }
      if ("+-*/()".includes(ch)) {
        result.push(ch);
      }
    }
  }
  if (current !== "") {
    result.push(current);
  }
  return result;
}

function isNumber(token) {
  return /^[\d.]+$/.test(token);
}

function formatExpression(expr) {
  return expr
    .replace(/\*/g, " × ")
    .replace(/\//g, " ÷ ")
    .replace(/\+/g, " + ")
    .replace(/-/g, " − ")
    .replace(/\(/g, "( ")
    .replace(/\)/g, " )")
    .replace(/  +/g, " ")
    .trim();
}

function formatResult(num) {
  if (typeof num === "string") return num;
  if (Number.isInteger(num)) return String(num);
  return parseFloat(num.toFixed(8)).toString();
}

function formatTime(date) {
  var h = String(date.getHours()).padStart(2, "0");
  var m = String(date.getMinutes()).padStart(2, "0");
  var s = String(date.getSeconds()).padStart(2, "0");
  return h + ":" + m + ":" + s;
}

function escapeHtml(str) {
  return str.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

function addStep(text, type) {
  var div = document.createElement("div");
  div.className = "step-item";
  if (type === "result") div.className += " result";
  if (type === "error") div.className += " error";
  div.textContent = text;
  stepsEl.appendChild(div);
  stepsEl.scrollTop = stepsEl.scrollHeight;
}

// ===== 鍵盤支援 =====
document.addEventListener("keydown", function (e) {
  var key = e.key;
  if ("0123456789".includes(key)) {
    inputChar(key);
  } else if (key === "+") {
    inputChar("+");
  } else if (key === "-") {
    inputChar("-");
  } else if (key === "*") {
    inputChar("*");
  } else if (key === "/") {
    e.preventDefault();
    inputChar("/");
  } else if (key === ".") {
    inputChar(".");
  } else if (key === "(") {
    inputChar("(");
  } else if (key === ")") {
    inputChar(")");
  } else if (key === "Enter" || key === "=") {
    e.preventDefault();
    calculate();
  } else if (key === "Backspace") {
    backspace();
  } else if (key === "Escape" || key === "Delete") {
    clearAll();
  }
});
